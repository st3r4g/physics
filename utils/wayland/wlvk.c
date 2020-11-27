#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define handle_error(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while(0)

struct state {
    struct wl_display *display;
    struct wl_compositor *compositor;
    struct xdg_wm_base *xdg_wm_base;
    struct wl_surface *surface;
    void (*render_callback)(int);
    uint32_t last_frame;
};

static const struct wl_callback_listener wl_surface_frame_listener;

static void
wl_surface_frame_done(void *data, struct wl_callback *cb, uint32_t time)
{
	/* Destroy this callback */
	wl_callback_destroy(cb);

	/* Request another frame */
	struct state *state = data;
	cb = wl_surface_frame(state->surface);
	wl_callback_add_listener(cb, &wl_surface_frame_listener, state);

	int elapsed = 0;
	if (state->last_frame != 0)
		elapsed = time - state->last_frame;

	/* Submit a frame for this event */
        state->render_callback(elapsed);

	state->last_frame = time;
}

static const struct wl_callback_listener wl_surface_frame_listener = {
	.done = wl_surface_frame_done,
};

static void
xdg_surface_configure(void *data,
        struct xdg_surface *xdg_surface, uint32_t serial)
{
    struct state *state = data;
    xdg_surface_ack_configure(xdg_surface, serial);
    /*
     * I think it is necessary to commit the first frame here otherwise
     * we don't receive the frame callback
     */
    state->render_callback(0);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void
registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version)
{
    struct state *state = data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        state->compositor = wl_registry_bind(
            registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        state->xdg_wm_base = wl_registry_bind(
            registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_wm_base,
                                 &xdg_wm_base_listener, state);
    }
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name)
{
	// This space deliberately left blank
}

static const struct wl_registry_listener
registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

static struct state state;

void wlvk_init(void (*render_callback)()) {
    state.display = wl_display_connect(NULL);
    if (!state.display)
        handle_error("wl_display_connect");
    struct wl_registry *registry = wl_display_get_registry(state.display);
    wl_registry_add_listener(registry, &registry_listener, &state);
    wl_display_roundtrip(state.display);
    state.surface = wl_compositor_create_surface(state.compositor);
    struct xdg_surface *xdg_surface = xdg_wm_base_get_xdg_surface(
            state.xdg_wm_base, state.surface);
    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, &state);
    struct xdg_toplevel *xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
    xdg_toplevel_set_title(xdg_toplevel, "wlvk");

    state.render_callback = render_callback;
    struct wl_callback *cb = wl_surface_frame(state.surface);
    wl_callback_add_listener(cb, &wl_surface_frame_listener, &state);

    wl_surface_commit(state.surface);
}

void wlvk_run() {
    while (wl_display_dispatch(state.display) != -1) {}
    wl_display_disconnect(state.display);
}

struct wl_display *wlvk_get_wl_display() {
	return state.display;
}

struct wl_surface *wlvk_get_wl_surface() {
	return state.surface;
}
