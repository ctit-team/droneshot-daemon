#include "uv.h"
#include "hardware/interface.h"
#include "hardware/transmitter_collection.h"
#include "setting/main.h"
#include "rpc/server.h"

#include <uv.h>

#ifdef DRONESHOT_SYSTEMD_SERVICE
#include <systemd/sd-daemon.h>
#endif

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void force_close(uv_handle_t *handle, void *arg)
{
	const struct uv_type *t;

	if (uv_is_closing(handle)) {
		return;
	}

	t = (const struct uv_type *)handle->data;
	if (t && t->destroy) {
		t->destroy(handle);
	} else {
		uv_close(handle, NULL);
	}
}

static void interrupt_handler(uv_signal_t *handle, int signum)
{
	uv_walk(handle->loop, force_close, NULL);
}

static bool listen_interrupt(uv_loop_t *uv, uv_signal_t *sig)
{
	int err;

	// initialize handle.
	err = uv_signal_init(uv, sig);
	if (err < 0) {
		fprintf(stderr, "Failed to initialize interrupt handle: %s.\n", uv_strerror(err));
		return false;
	}

	sig->data = NULL;

	// start watching signal.
	err = uv_signal_start_oneshot(sig, interrupt_handler, SIGINT);
	if (err < 0) {
		fprintf(stderr, "Failed to listen for interrupt signal: %s.\n", uv_strerror(err));
		uv_close((uv_handle_t *)sig, NULL);
		return false;
	}

	return true;
}

static bool run(void)
{
	bool res = true;
	int err;
	uv_loop_t uv;
	uv_signal_t sig;
	struct rpc_server *s;

	// initialize libuv.
	err = uv_loop_init(&uv);
	if (err < 0) {
		fprintf(stderr, "Failed to initializes libuv: %s.\n", uv_strerror(err));
		return false;
	}

	// listen for interrupt signal
	if (!listen_interrupt(&uv, &sig)) {
		res = false;
		goto run;
	}

	// start RPC server.
	s = rpc_server_new(&uv, "/tmp/droneshot");
	if (!s) {
		goto fail_with_signal;
	}

	if (!rpc_server_start(s)) {
		goto fail_with_server;
	}

#ifdef DRONESHOT_SYSTEMD_SERVICE
	// notify systemd that we are ready.
	err = sd_notify(false, "READY=1");
	if (err < 0) {
		fprintf(stderr, "Failed to notify systemd that we are ready: %s.\n", strerror(-err));
		goto fail_with_server;
	}
#endif

	// run libuv.
	run:

	err = uv_run(&uv, UV_RUN_DEFAULT);
	if (err < 0) {
		fprintf(stderr, "Failed to run libuv: %s.\n", uv_strerror(err));
		res = false;
	}

	// clean up.
	err = uv_loop_close(&uv);
	if (err < 0) {
		fprintf(stderr, "Failed to shutdown libuv: %s.\n", uv_strerror(err));
	}

	return res;

	// error handlers.
	fail_with_server:
	rpc_server_free(s);

	fail_with_signal:
	uv_close((uv_handle_t *)&sig, NULL);

	res = false;
	goto run;
}

static bool init_hardware(void)
{
	if (hardware_interface_init()) {
		if (transmitter_collection_init()) {
			return true;
		}

		hardware_interface_close();
	}

	return false;
}

static void close_hardware()
{
	transmitter_collection_close();
	hardware_interface_close();
}

static bool init_settings(void)
{
	if (!setting_main_init()) {
		return false;
	}

	return true;
}

static void term_settings(void)
{
}

int main(int argc, char *argv[])
{
	int res;

	if (!init_settings()) {
		return EXIT_FAILURE;
	}

	res = EXIT_FAILURE;

	if (init_hardware()) {
		if (run()) {
			res = EXIT_SUCCESS;
		}

		close_hardware();
	}

	term_settings();

	return res;
}
