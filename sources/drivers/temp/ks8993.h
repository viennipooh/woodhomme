//******************************************************************************
// ��� �����    :       ks8993.h
// ���������    :       ������� ������ KS8993
// �����        :       �������� �.�.
// ����         :       18.11.2015
//
//------------------------------------------------------------------------------
//      
//      int ks8993_init( void ); // ���������� ������ ����������������
//      ���������� �������� ����������������� ������ �� ����� ������ MAC MII
//******************************************************************************
#ifndef __KS8993_H_
#define __KS8993_H_


/********************************************
 * ����� ������������ ����������� �������
 * enum vlan_group_e -> vlan_group_t
 * -----------------------------------------
 * ->VLAN_G123
 *                  3 (MCU)
 *                /  \
 *               1 -  2
 * -----------------------------------------
 * ->VLAN_G13_G23
 *                  3 (MCU)
 *                /  \
 *               1    2
 * -----------------------------------------
 * ->VLAN_G13_G12
 *                  3 (MCU)
 *                /   
 *               1 -  2
 * -----------------------------------------
 * ->VLAN_G12_G23
 *                  3 (MCU)
 *                   \
 *               1 -  2
 * -----------------------------------------
 ********************************************/    
enum ks8993_vlan_group_e {
  VLAN_G123,
  VLAN_G13_G23,
  VLAN_G13_G12,
  VLAN_G12_G23,
};
typedef enum ks8993_vlan_group_e ks8993_vlan_group_t;

enum ks8993_mii_mode_e {
  MII_DISABLE,
  MII_REVERSE,
  MII_EXT_MAC_MODE,
  MII_FORWARD,
  MII_EXT_PHY_MODE,
  MII_SNI,
};
typedef enum ks8993_mii_mode_e ks8993_mii_mode_t;

enum ks8993_port_e {
  PORT_1,
  PORT_2,
  PORT_3,
  P1 = PORT_1,
  P2 = PORT_2,
  P3 = PORT_3,
};
typedef enum ks8993_port_e ks8993_port_t;

enum ks8993_flow_control_e {
  ENABLE_FLOW_CONTROL,
  DISABLE_FLOW_CONTROL,
};
typedef enum ks8993_flow_control_e ks8993_flow_control_t;
  
enum ks8993_led_mode_e {
  LED_MODE_0,
  LED_MODE_1,
  LED_MODE_2,
  LED_MODE_3,
  LED_MODE_0_EXT,
  LED_MODE_2_EXT,
  LED_MODE_3_EXT,
};
typedef enum ks8993_led_mode_e ks8993_led_mode_t;


enum ks8993_speed_e {
  SPEED_10,
  SPEED_100,
};
typedef enum ks8993_speed_e ks8993_speed_t;

enum ks8993_duplex_e {
  HALF_DUPLEX,
  FULL_DUPLEX,
  HD = HALF_DUPLEX,
  FD = FULL_DUPLEX,
};
typedef enum ks8993_duplex_e ks8993_duplex_t;

enum ks8993_autonegotiation_e {
  AUTO_ENABLE,
  AUTO_DISABLE,
};
typedef enum ks8993_autonegotiation_e ks8993_autonegotiation_t;


struct ks8993_s {
  ks8993_vlan_group_t port_group;
  ks8993_mii_mode_t   mii_mode;
};
typedef struct ks8993_s ks8993_t;

//==============================================================================
//                      ������������� ������
//   ������������� ����������� ������������ ����� ��������� ��� ���������
// ����������� ���������� ������.
//   ��� ������������� ����������� �������� �����.
//   ������� ������������� ������ ������� ������������� ������ � ����� ������
// �������, ���� ���.
//------------------------------------------------------------------------------
// return       -       0 ��� �������� ����������
//==============================================================================
int ks8993_init( void ); // ���������� ������ ����������������

/* ����� ������ */
int ks8993_reset_on( void );

/* ������������� ����������������� ������, � ����������� ��������,
   ������������ �� �����  */
int ks8993_reset_off( void );

/* ����� ����������� �������, ����� ��������� ������� ���������� � ��������
   ������������ enum ks8993_vlan_group_e.
   ��������� ������������ ���������� �����������, 
    ������������� ������ ������ ���*/
int ks8993_select_group(ks8993_vlan_group_t); /* ������� ����� � ����� ����� */

/* ����� ������ MII �����, �� ��������� ���������� � ks8993_init*/
int ks8993_select_mode_mii(ks8993_mii_mode_t); /* ��������� reset */
/* ����� ������ ������ LED �����������, �� ��������� ���������� � ks8993_init*/
int ks8993_select_mode_led(ks8993_led_mode_t); /* ��������� reset */

/* ����� ������ ������ LED �����������, �� ��������� ���������� � ks8993_init*/
int ks8993_select_flow_control(ks8993_port_t,ks8993_flow_control_t);/* ��������� reset ��� 3-��� �����*/
int ks8993_select_duplex(ks8993_port_t,ks8993_duplex_t);/* ��������� reset*/
int ks8993_select_speed(ks8993_port_t,ks8993_speed_t);/* ��������� reset*/
int ks8993_select_autonegotiation(ks8993_port_t,ks8993_autonegotiation_t);/* ��������� reset */

int ks8993_select_flow_control_in_auto(int enable);/* ��������� reset*/
int ks8993_select_buffer_per_port_allocation(int enable);
int ks8993_select_mac_table_aging(int enable);
int ks8993_select_back_pressure_at_reset(int enable);
int ks8993_select_back_off_hf_at_reset(int enable);
int ks8993_select_no_excessive_collision_drop_at_reset(int enable);

int ks8993_deinit( void ); /* �� ������� ������ �� ������������ */



#endif //__KS8993_H_