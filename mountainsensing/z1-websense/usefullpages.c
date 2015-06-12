/* This file contains usefull pages that cam be made avaliable from a webserver

 * Routes, Prints the routes and neighbours the node has
 */

#include "contiki.h"
#include "httpd-simple.h"
#include "webserver-nogui.h"
#include "dev/temperature-sensor.h"
#include "dev/battery-sensor.h"

#ifndef CC11xx_CC1120
#include "dev/cc2420.h"
#endif

#include "dev/leds.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
static const char *TOP = "<html><head><title>Mountain Sensing</title></head><body>\n";
static const char *BOTTOM = "</body></html>\n";
/*---------------------------------------------------------------------------*/

/* Only one single request at time */
static char buf[256];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)


/* generate a chart using google apis
 *
 * /param title char array of title of graph
 * /param unit char array of graph unit
 * /param min minimum value for the graph
 * /param max maximum value for the graph
 * /param values array of values to graph
 */
#ifdef LALALALA
static void
generate_chart(const char *title, const char *unit, int min, int max, int *values)
{
  int i;
  blen = 0;
  ADD("<h1>%s</h1>\n"
      "<img src=\"http://chart.apis.google.com/chart?"
      "cht=lc&chs=400x300&chxt=x,x,y,y&chxp=1,50|3,50&"
      "chxr=2,%d,%d|0,0,30&chds=%d,%d&chxl=1:|Time|3:|%s&chd=t:",
      title, min, max, min, max, unit);
  for(i = 0; i < HISTORY; i++) {
    ADD("%s%d", i > 0 ? "," : "", values[(sensors_pos + i) % HISTORY]);
  }
  ADD("\">");
}

//old webserver code
static
PT_THREAD(send_values(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, TOP);

  if(s->filename[1] == '0') {
    /* Turn off leds */
    leds_off(LEDS_ALL);
    SEND_STRING(&s->sout, "Turned off leds!");

  } else if(s->filename[1] == '1') {
    /* Turn on leds */
    leds_on(LEDS_ALL);
    SEND_STRING(&s->sout, "Turned on leds!");

  } else {
    if(s->filename[1] != 't') {
      generate_chart("Battery", "mV", 0, 4000, battery1);
      SEND_STRING(&s->sout, buf);
    }
    if(s->filename[1] != 'b') {
      generate_chart("Temperature", "Celsius", 0, 50, temperature);
      SEND_STRING(&s->sout, buf);
    }
  }

  SEND_STRING(&s->sout, BOTTOM);

  PSOCK_END(&s->sout);
}
#endif

static void
ipaddr_add(const uip_ipaddr_t *addr)
{
  uint16_t a;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) ADD("::");
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        ADD(":");
      }
      ADD("%x", a);
    }
  }
}


/* Sends the routes avalaible to the node out of the webserver socket
 */
static
PT_THREAD(webserver_routes(struct httpd_state *s))
{
  static int i;
  static uip_ds6_route_t *r;
  static uip_ds6_nbr_t *nbr;

#ifdef WEBSERVER_CONF_LOADTIME
  static clock_time_t numticks;
  numticks = clock_time();
#endif

  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, TOP);
  blen = 0;
  
/*
 * print neighbour table
 */

  ADD("Neighbors<pre>");

  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = nbr_table_next(ds6_neighbors, nbr)) {

      {uint8_t j=blen+25;
      ipaddr_add(&nbr->ipaddr);
      while (blen < j) ADD(" ");
        switch (nbr->state) {
          case NBR_INCOMPLETE: ADD(" INCOMPLETE");break;
          case NBR_REACHABLE: ADD(" REACHABLE");break;
          case NBR_STALE: ADD(" STALE");break;      
          case NBR_DELAY: ADD(" DELAY");break;
          case NBR_PROBE: ADD(" NBR_PROBE");break;
        }
      }
      ADD("\n");

      if(blen > sizeof(buf) - 45) {
        SEND_STRING(&s->sout, buf);
        blen = 0;
      }
  }

/*
 * print route table
 */

  ADD("</pre>Routes<pre>");
  SEND_STRING(&s->sout, buf);
  blen = 0;

  for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {

    ADD("<a href=http://[");
    ipaddr_add(&r->ipaddr);
    ADD("]/status.shtml>");
    SEND_STRING(&s->sout, buf); //TODO: why tunslip6 needs an output here, wpcapslip does not
    blen = 0;
    ipaddr_add(&r->ipaddr);
    ADD("</a>");
    
    ADD("/%u (via ", r->length);
    ipaddr_add(uip_ds6_route_nexthop(r));
    if(1 || (r->state.lifetime < 600)) {
      ADD(") %lus\n", r->state.lifetime);
    } else {
      ADD(")\n");
    }
    SEND_STRING(&s->sout, buf);
    blen = 0;
  }
  ADD("</pre>");

/*
 * Print extra info about the page
 */

#ifdef WEBSERVER_CONF_FILESTATS
  static uint16_t numtimes;
  ADD("<br><i>This page sent %u times</i>",++numtimes);
#endif

#ifdef WEBSERVER_CONF_LOADTIME
  numticks = clock_time() - numticks + 1;
  ADD(" <i>(%u.%02u sec)</i>",numticks/CLOCK_SECOND,(100*(numticks%CLOCK_SECOND))/CLOCK_SECOND));
#endif

  SEND_STRING(&s->sout, buf);
  SEND_STRING(&s->sout, BOTTOM);

  PSOCK_END(&s->sout);
}
