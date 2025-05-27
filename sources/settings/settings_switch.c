#include "settings.h"
#include "extio.h"
#include "info.h"
#ifdef USE_SWITCH //Использовать свитч kmz8895
  #include "switch.h"

#endif

void settings_load_switch(struct switch_settings_s* sw)
{
  for(int i=0;i<PORT_NUMBER;i++)
  {
    // !!! Только разрешение приема/передачи
    switch_port_state(i, sw->ports[i].port_enabled);
    switch_select_duplex(i,sw->ports[i].full_duplex);
    switch_select_flow_control(i,sw->ports[i].flow_control);
		switch_select_autonegotiation(PORT1,sw->ports[i].autonegotiation);
		switch_select_speed(ETH1,sw->ports[i].speed);
		switch_select_flow_control(ETH1,sw->ports[i].flow_control);
  }

#ifdef TWO_SWITCH_ITEMS
  if(   sw->ports[OT3].port_enabled ==0 &&
        sw->ports[OT4].port_enabled ==0 &&
        sw->ports[ETH6].port_enabled ==0 &&
        sw->ports[ETH7].port_enabled ==0 )
  {
    if(rsettings->swpwr)
    {
      vTaskDelay(500);
      switch_on(1);
    }
    else
    {
      switch_off(1);
    }
  }

  switch_select_autonegotiation(PORT1,sw->ports[0].autonegotiation);
  switch_select_autonegotiation(PORT2,sw->ports[1].autonegotiation);
  switch_select_autonegotiation(PORT3,sw->ports[2].autonegotiation);
  switch_select_autonegotiation(PORT4,sw->ports[3].autonegotiation);

  switch_select_autonegotiation(ETH5,sw->ports[4].autonegotiation);
  switch_select_autonegotiation(ETH6,sw->ports[5].autonegotiation);
  switch_select_autonegotiation(ETH7,sw->ports[6].autonegotiation);

  switch_select_speed(ETH5,sw->ports[4].speed);
  switch_select_speed(ETH6,sw->ports[5].speed);
  switch_select_speed(ETH7,sw->ports[6].speed);


  switch_select_flow_control(ETH5,sw->ports[4].flow_control);
  switch_select_flow_control(ETH6,sw->ports[5].flow_control);
  switch_select_flow_control(ETH7,sw->ports[6].flow_control);
#endif
}