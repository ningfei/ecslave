
/*********************** net proto  start **************************/
#define PF_ETHERCAT PF_ECONET

/*
 *	Create an ethercat socket
 */
static int ecat_create(struct net *net, struct socket *sock, int protocol,
			 int kern)
{
	struct sock *sk;
	struct ethercat_sock *ecat_sock;
	int err;

	if (!net_eq(net, &init_net))
		return -EAFNOSUPPORT;

	/* ethercat only provides datagram services. */
	if (sock->type != SOCK_DGRAM)
		return -ESOCKTNOSUPPORT;

	sock->state = SS_UNCONNECTED;

	err = -ENOBUFS;
	sk = sk_alloc(net,  PF_ETHERCAT, GFP_KERNEL, &ecat_proto);
	if (sk == NULL)
		goto out;

	sk->sk_reuse = 1;
	sock->ops = &ecat_ops;
	sock_init_data(sock, sk);

	ecat_sock = ec_sk(sk);
	sock_reset_flag(sk, SOCK_ZAPPED);
	sk->sk_family = PF_ETHERCAT;
	ecat_sock->num = protocol;

	ecat_insert_socket(&ecat_sklist, sk);
	return 0;
out:
	return err;
}

static const struct net_proto_family ecat_family_ops = {
	.family =	PF_ECONET,
	.create =	ecat_create,
	.owner	=	THIS_MODULE,
};

