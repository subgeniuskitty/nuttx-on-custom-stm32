/****************************************************************************
 * net/devif/devif_poll.c
 *
 *   Copyright (C) 2007-2010, 2012, 2014, 2016 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#ifdef CONFIG_NET

#include <debug.h>

#include <nuttx/clock.h>
#include <nuttx/net/netconfig.h>
#include <nuttx/net/netdev.h>
#include <nuttx/net/net.h>

#include "devif/devif.h"
#include "arp/arp.h"
#include "neighbor/neighbor.h"
#include "tcp/tcp.h"
#include "udp/udp.h"
#include "pkt/pkt.h"
#include "icmp/icmp.h"
#include "icmpv6/icmpv6.h"
#include "igmp/igmp.h"
#include "sixlowpan/sixlowpan.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

enum devif_packet_type
{
  DEVIF_PKT = 0,
  DEVIF_ICMP,
  DEVIF_IGMP,
  DEVIF_TCP,
  DEVIF_UDP,
  DEVIF_ICMP6
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* Time of last poll */

systime_t g_polltime;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: devif_packet_conversion
 *
 * Description:
 *   Generic output conversion hook.  Only needed for IEEE802.15.4 for now
 *   is a point where support for other conversions may be provided.
 *
 *   TCP output comes through three different mechansims.  Either from:
 *
 *   1. TCP socket output.  For the case of TCP output to an
 *      IEEE802.15.4, the TCP output is caught in the socket
 *      send()/sendto() logic and and redirected to 6loWPAN logic.
 *   2. TCP output from the TCP state machine.  That will occur
 *      during TCP packet processing by the TCP state meachine.
 *   3. TCP output resulting from TX or timer polling
 *
 *   Cases 2 is handled here.  Logic here detected if (1) an attempt
 *   to return with d_len > 0 and (2) that the device is an
 *   IEEE802.15.4 MAC network driver. Under those conditions, 6loWPAN
 *   logic will be called to create the IEEE80215.4 frames.
 *
 * Assumptions:
 *   The network is locked.
 *
 ****************************************************************************/

#ifdef CONFIG_NET_6LOWPAN
static void devif_packet_conversion(FAR struct net_driver_s *dev,
                                    enum devif_packet_type pkttype)
{
#ifdef CONFIG_NET_MULTILINK
  /* Handle the case where multiple link layer protocols are supported */

  if (dev->d_len > 0 && dev->d_lltype == NET_LL_IEEE802154)
#else
  if (dev->d_len > 0)
#endif
    {
      if (pkttype == DEVIF_TCP)
        {
          FAR struct ipv6_hdr_s *ipv6 = (FAR struct ipv6_hdr_s *)dev->d_buf;

          /* This packet came from a response to TCP polling and is directed
           * to an IEEE802.15.4 device using 6loWPAN.  Verify that the outgoing
           * packet is IPv6 with TCP protocol.
           */

          if (ipv6->vtc ==  IPv6_VERSION && ipv6->proto == IP_PROTO_TCP)
            {
              /* Let 6loWPAN convert IPv6 TCP output into IEEE802.15.4 frames. */

              sixlowpan_tcp_send(dev);
            }
          else
            {
              nerr("ERROR: IPv6 version or protocol error.  Packet dropped\n");
              nerr("       IP version: %02x proocol: %u\n",
                   ipv6->vtc, ipv6->proto);
            }
        }
      else
        {
          nerr("ERROR: Non-TCP packet dropped.  Packet type: %u\n", pkttype);
        }

      dev->d_len = 0;
    }
}
#else
# define devif_packet_conversion(dev,pkttype)
#endif /* CONFIG_NET_6LOWPAN */

/****************************************************************************
 * Name: devif_poll_pkt_connections
 *
 * Description:
 *   Poll all packet connections for available packets to send.
 *
 * Assumptions:
 *   This function is called from the MAC device driver and may be called
 *   from the timer interrupt/watchdog handle level.
 *
 ****************************************************************************/

#ifdef CONFIG_NET_PKT
static int devif_poll_pkt_connections(FAR struct net_driver_s *dev,
                                      devif_poll_callback_t callback)
{
  FAR struct pkt_conn_s *pkt_conn = NULL;
  int bstop = 0;

  /* Traverse all of the allocated packet connections and perform the poll action */

  while (!bstop && (pkt_conn = pkt_nextconn(pkt_conn)))
    {
      /* Perform the packet TX poll */

      pkt_poll(dev, pkt_conn);

      /* Perform any necessary conversions on outgoing packets */

      devif_packet_conversion(dev, DEVIF_PKT);

      /* Call back into the driver */

      bstop = callback(dev);
    }

  return bstop;
}
#endif /* CONFIG_NET_PKT */

/****************************************************************************
 * Name: devif_poll_icmp
 *
 * Description:
 *   Poll all of the connections waiting to send an ICMP ECHO request
 *
 ****************************************************************************/

#if defined(CONFIG_NET_ICMP) && defined(CONFIG_NET_ICMP_PING)
static inline int devif_poll_icmp(FAR struct net_driver_s *dev,
                                  devif_poll_callback_t callback)
{
  /* Perform the ICMP poll */

  icmp_poll(dev);

  /* Perform any necessary conversions on outgoing packets */

  devif_packet_conversion(dev, DEVIF_ICMP);

  /* Call back into the driver */

  return callback(dev);
}
#endif /* CONFIG_NET_ICMP && CONFIG_NET_ICMP_PING */

/****************************************************************************
 * Name: devif_poll_icmpv6
 *
 * Description:
 *   Poll all of the connections waiting to send an ICMPv6 ECHO request
 *
 ****************************************************************************/

#if defined(CONFIG_NET_ICMPv6_PING) || defined(CONFIG_NET_ICMPv6_NEIGHBOR)
static inline int devif_poll_icmpv6(FAR struct net_driver_s *dev,
                                    devif_poll_callback_t callback)
{
  /* Perform the ICMPv6 poll */

  icmpv6_poll(dev);

  /* Perform any necessary conversions on outgoing packets */

  devif_packet_conversion(dev, DEVIF_ICMP6);

  /* Call back into the driver */

  return callback(dev);
}
#endif /* CONFIG_NET_ICMPv6_PING || CONFIG_NET_ICMPv6_NEIGHBOR*/

/****************************************************************************
 * Name: devif_poll_igmp
 *
 * Description:
 *   Poll all IGMP connections for available packets to send.
 *
 * Assumptions:
 *   This function is called from the MAC device driver and may be called
 *   from the timer interrupt/watchdog handle level.
 *
 ****************************************************************************/

#ifdef CONFIG_NET_IGMP
static inline int devif_poll_igmp(FAR struct net_driver_s *dev,
                                  devif_poll_callback_t callback)
{
  /* Perform the IGMP TX poll */

  igmp_poll(dev);

  /* Perform any necessary conversions on outgoing packets */

  devif_packet_conversion(dev, DEVIF_IGMP);

  /* Call back into the driver */

  return callback(dev);
}
#endif /* CONFIG_NET_IGMP */

/****************************************************************************
 * Name: devif_poll_udp_connections
 *
 * Description:
 *   Poll all UDP connections for available packets to send.
 *
 * Assumptions:
 *   This function is called from the MAC device driver and may be called
 *   from the timer interrupt/watchdog handle level.
 *
 ****************************************************************************/

#ifdef NET_UDP_HAVE_STACK
static int devif_poll_udp_connections(FAR struct net_driver_s *dev,
                                      devif_poll_callback_t callback)
{
  FAR struct udp_conn_s *conn = NULL;
  int bstop = 0;

  /* Traverse all of the allocated UDP connections and perform the poll action */

  while (!bstop && (conn = udp_nextconn(conn)))
    {
      /* Perform the UDP TX poll */

      udp_poll(dev, conn);

      /* Perform any necessary conversions on outgoing packets */

      devif_packet_conversion(dev, DEVIF_UDP);

      /* Call back into the driver */

      bstop = callback(dev);
    }

  return bstop;
}
#endif /* NET_UDP_HAVE_STACK */

/****************************************************************************
 * Name: devif_poll_tcp_connections
 *
 * Description:
 *   Poll all UDP connections for available packets to send.
 *
 * Assumptions:
 *   This function is called from the MAC device driver and may be called
 *   from the timer interrupt/watchdog handle level.
 *
 ****************************************************************************/

#ifdef NET_TCP_HAVE_STACK
static inline int devif_poll_tcp_connections(FAR struct net_driver_s *dev,
                                             devif_poll_callback_t callback)
{
  FAR struct tcp_conn_s *conn  = NULL;
  int bstop = 0;

  /* Traverse all of the active TCP connections and perform the poll action */

  while (!bstop && (conn = tcp_nextconn(conn)))
    {
      /* Perform the TCP TX poll */

      tcp_poll(dev, conn);

      /* Perform any necessary conversions on outgoing packets */

      devif_packet_conversion(dev, DEVIF_TCP);

      /* Call back into the driver */

      bstop = callback(dev);
    }

  return bstop;
}
#else
# define devif_poll_tcp_connections(dev, callback) (0)
#endif

/****************************************************************************
 * Name: devif_poll_tcp_timer
 *
 * Description:
 *   The TCP timer has expired.  Update TCP timing state in each active,
 *   TCP connection.
 *
 * Assumptions:
 *   This function is called from the MAC device driver and may be called
 *   from the timer interrupt/watchdog handle level.
 *
 ****************************************************************************/

#ifdef NET_TCP_HAVE_STACK
static inline int devif_poll_tcp_timer(FAR struct net_driver_s *dev,
                                       devif_poll_callback_t callback,
                                       int hsec)
{
  FAR struct tcp_conn_s *conn  = NULL;
  int bstop = 0;

  /* Traverse all of the active TCP connections and perform the poll action. */

  while (!bstop && (conn = tcp_nextconn(conn)))
    {
      /* Perform the TCP timer poll */

      tcp_timer(dev, conn, hsec);

      /* Perform any necessary conversions on outgoing packets */

      devif_packet_conversion(dev, DEVIF_TCP);

      /* Call back into the driver */

      bstop = callback(dev);
    }

  return bstop;
}
#else
# define devif_poll_tcp_timer(dev, callback, hsec) (0)
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: devif_poll
 *
 * Description:
 *   This function will traverse each active network connection structure and
 *   will perform network polling operations. devif_poll() may be called
 *   asynchronously with the network driver can accept another outgoing
 *   packet.
 *
 *   This function will call the provided callback function for every active
 *   connection. Polling will continue until all connections have been polled
 *   or until the user-supplied function returns a non-zero value (which it
 *   should do only if it cannot accept further write data).
 *
 *   When the callback function is called, there may be an outbound packet
 *   waiting for service in the device packet buffer, and if so the d_len field
 *   is set to a value larger than zero. The device driver should then send
 *   out the packet.
 *
 * Assumptions:
 *   This function is called from the MAC device driver and may be called
 *   from the timer interrupt/watchdog handle level.
 *
 ****************************************************************************/

int devif_poll(FAR struct net_driver_s *dev, devif_poll_callback_t callback)
{
  int bstop = false;

  /* Traverse all of the active packet connections and perform the poll
   * action.
   */

#ifdef CONFIG_NET_ARP_SEND
  /* Check for pending ARP requests */

  bstop = arp_poll(dev, callback);
  if (!bstop)
#endif
#ifdef CONFIG_NET_PKT
    {
      /* Check for pending packet socket transfer */

      bstop = devif_poll_pkt_connections(dev, callback);
    }

  if (!bstop)
#endif
#ifdef CONFIG_NET_IGMP
    {
      /* Check for pending IGMP messages */

      bstop = devif_poll_igmp(dev, callback);
    }

  if (!bstop)
#endif
#ifdef NET_TCP_HAVE_STACK
    {
      /* Traverse all of the active TCP connections and perform the poll
       * action.
       */

      bstop = devif_poll_tcp_connections(dev, callback);
    }

  if (!bstop)
#endif
#ifdef NET_UDP_HAVE_STACK
    {
      /* Traverse all of the allocated UDP connections and perform
       * the poll action
       */

      bstop = devif_poll_udp_connections(dev, callback);
    }

  if (!bstop)
#endif
#if defined(CONFIG_NET_ICMP) && defined(CONFIG_NET_ICMP_PING)
    {
      /* Traverse all of the tasks waiting to send an ICMP ECHO request. */

      bstop = devif_poll_icmp(dev, callback);
    }

  if (!bstop)
#endif
#if defined(CONFIG_NET_ICMPv6_PING) || defined(CONFIG_NET_ICMPv6_NEIGHBOR)
    {
      /* Traverse all of the tasks waiting to send an ICMPv6 ECHO request. */

      bstop = devif_poll_icmpv6(dev, callback);
    }

  if (!bstop)
#endif
    {
      /* Nothing more to do */
    }

  return bstop;
}

/****************************************************************************
 * Name: devif_timer
 *
 * Description:
 *   These function will traverse each active network connection structure and
 *   perform network timer operations. The Ethernet driver MUST implement
 *   logic to periodically call devif_timer().
 *
 *   This function will call the provided callback function for every active
 *   connection. Polling will continue until all connections have been polled
 *   or until the user-supplied function returns a non-zero value (which it
 *   should do only if it cannot accept further write data).
 *
 *   When the callback function is called, there may be an outbound packet
 *   waiting for service in the device packet buffer, and if so the d_len field
 *   is set to a value larger than zero. The device driver should then send
 *   out the packet.
 *
 * Assumptions:
 *   This function is called from the MAC device driver and may be called from
 *   the timer interrupt/watchdog handle level.
 *
 ****************************************************************************/

int devif_timer(FAR struct net_driver_s *dev, devif_poll_callback_t callback)
{
  systime_t now;
  systime_t elapsed;
  int bstop = false;

  /* Get the elapsed time since the last poll in units of half seconds
   * (truncating).
   */

  now     = clock_systimer();
  elapsed = now - g_polltime;

  /* Process time-related events only when more than one half second elapses. */

  if (elapsed >= TICK_PER_HSEC)
    {
      /* Calculate the elpased time in units of half seconds (truncating to
       * number of whole half seconds).
       */

      int hsec = (int)(elapsed / TICK_PER_HSEC);

      /* Update the current poll time (truncating to the last half second
       * boundary to avoid error build-up).
       */

      g_polltime += (TICK_PER_HSEC * (systime_t)hsec);

      /* Perform periodic activitives that depend on hsec > 0 */

#if defined(CONFIG_NET_TCP_REASSEMBLY) && defined(CONFIG_NET_IPv4)
      /* Increment the timer used by the IP reassembly logic */

      if (g_reassembly_timer != 0 &&
          g_reassembly_timer < CONFIG_NET_TCP_REASS_MAXAGE)
        {
          g_reassembly_timer += hsec;
        }
#endif

#ifdef CONFIG_NET_IPv6
      /* Perform aging on the entries in the Neighbor Table */

       neighbor_periodic(hsec);
#endif

#ifdef NET_TCP_HAVE_STACK
      /* Traverse all of the active TCP connections and perform the
       * timer action.
       */

      bstop = devif_poll_tcp_timer(dev, callback, hsec);
#endif
    }

  /* If possible, continue with a normal poll checking for pending
   * network driver actions.
   */

  if (!bstop)
    {
      bstop = devif_poll(dev, callback);
    }

  return bstop;
}

#endif /* CONFIG_NET */
