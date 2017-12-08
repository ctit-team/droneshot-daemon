#include "uv.h"
#include "hardware/interface.h"
#include "hardware/transmitter_collection.h"
#include "rpc/server.h"

#include <uv.h>

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void cleanup_handle(uv_handle_t *handle)
{
	const struct uv_type *t = handle->data;
	if (!t) {
		return;
	}

	t->cleanup(handle);
}

static void force_close(uv_handle_t *handle, void *arg)
{
	if (uv_is_closing(handle)) {
		return;
	}

	uv_close(handle, cleanup_handle);
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

	// initialize libuv.
	err = uv_loop_init(&uv);
	if (err < 0) {
		fprintf(stderr, "Failed to initializes libuv: %s.\n", uv_strerror(err));
		return false;
	}

	if (!listen_interrupt(&uv, &sig)) {
		res = false;
		goto run;
	}

	if (!rpc_server_start(&uv)) {
		res = false;
		uv_close((uv_handle_t *)&sig, NULL);
		goto run;
	}

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

int main(int argc, char *argv[])
{
	int res = EXIT_FAILURE;

	if (init_hardware()) {
		if (run()) {
			res = EXIT_SUCCESS;
		}

		close_hardware();
	}

	return res;
}
