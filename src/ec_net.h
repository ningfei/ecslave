#ifndef __EC_NET_H__
#define __EC_NET_H__

struct __ec_interface__;
struct __e_slave__;

#define RX_INT_INDEX	0	/* port in the master side */
#define TX_INT_INDEX	1	/* port in the next side  */

int ec_is_nic_link_up(struct __e_slave__ *, int eth);
int ec_is_nic_loop_closed( struct __e_slave__ *, int eth);
int ec_is_nic_signal_detected(struct __e_slave__ *, int eth);
struct __ec_interface__ * ec_tx_interface(struct __e_slave__ *);
struct __ec_interface__ * ec_rx_interface(struct __e_slave__ *);

#endif
