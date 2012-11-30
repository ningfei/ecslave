#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>

#include "xgeneral.h"
#include "globals.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_device.h"
#include "ec_debug.h"
#include "ecdev.h"

/*****************************************************************************/

int ec_device_init(ec_device_t * device,
			     /**< EtherCAT device */
		   ecat_node_t * ecat_node
			       /**< ethercat node owning the device */
    )
{
	device->ecat_node = ecat_node;
	ec_device_detach(device);	// resets remaining fields
	return 0;
}

/*****************************************************************************/

/** Associate with net_device.
 */
void ec_device_attach(ec_device_t * device,
			     /**< EtherCAT device */
		      struct net_device *net_dev
				   /**< net_device structure */
    )
{
	ec_device_detach(device);	// resets fields
	device->dev = net_dev;
}

/*****************************************************************************/
void ec_device_detach(ec_device_t * device
			    /**< EtherCAT device */
    )
{

	device->dev = NULL;
	ec_device_clear_stats(device);
}

/*****************************************************************************/

/** Clears the frame statistics.
 */
void ec_device_clear_stats(ec_device_t * device
			    /**< EtherCAT device */
    )
{
	device->tx_count = 0;
	device->rx_count = 0;
	device->tx_errors = 0;
	device->tx_bytes = 0;
	device->last_tx_count = 0;
	device->last_tx_bytes = 0;
	device->last_loss = 0;
}

/*****************************************************************************/
void ec_device_send(struct ec_device *device, struct sk_buff *skb)
{
	int ret;

	/* copy device's mac . better for debugging */
	memcpy(eth_hdr(skb)->h_source, device->dev->dev_addr,ETH_ALEN);
	ret =  device->dev->netdev_ops->ndo_start_xmit(skb, device->dev);
	if (ret == NETDEV_TX_OK) {
		device->tx_count++;
		return;
	}
	device->tx_errors++;
}
