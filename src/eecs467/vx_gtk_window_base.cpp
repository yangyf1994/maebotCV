#include <eecs467/vx_gtk_window_base.hpp>
#include <eecs467/vx_utils.h>
#include <vx/gtk/vx_gtk_display_source.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <unistd.h>

namespace eecs467
{
    
int handle_touch_event(vx_event_handler_t* vh, vx_layer_t* vl, vx_camera_pos_t* pos, vx_touch_event_t* mouse);
int handle_mouse_event(vx_event_handler_t* vh, vx_layer_t* vl, vx_camera_pos_t* pos, vx_mouse_event_t* mouse);
int handle_key_event(vx_event_handler_t* vh, vx_layer_t* vl, vx_key_event_t* key);
void destroy_event_handler(vx_event_handler_t* vh);

void* render_loop(void* data);
    

VxGtkWindowBase::VxGtkWindowBase(int argc, char** argv, int widthInPixels, int heightInPixels, int framesPerSecond)
: world_(0)
, displayImpl_(0)
, width_(widthInPixels)
, height_(heightInPixels)
, framesPerSecond_(framesPerSecond)
, isRunning_(false)
{
    assert(argc > 0);
    assert(argv);
    assert(widthInPixels > 0);
    assert(heightInPixels > 0);
    assert(framesPerSecond > 0);
    
    eecs467_init(argc, argv);
    
    world_ = vx_world_create();
    eventHandler_.key_event      = handle_key_event;
    eventHandler_.mouse_event    = handle_mouse_event;
    eventHandler_.touch_event    = handle_touch_event;
    eventHandler_.destroy        = destroy_event_handler;
    eventHandler_.dispatch_order = 100;
    eventHandler_.impl           = this;
    
    displayImpl_ = eecs467_default_implementation_create(world_, &eventHandler_);
    
    application_.display_started = eecs467_default_display_started;
    application_.display_finished = eecs467_default_display_finished;
    application_.impl = displayImpl_;
}


VxGtkWindowBase::~VxGtkWindowBase(void)
{
}


void VxGtkWindowBase::run(void)
{
    vx_gtk_display_source_t* displaySource = vx_gtk_display_source_create(&application_);
    GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    GtkWidget* canvas = vx_gtk_display_source_get_widget(displaySource);
    gtk_window_set_default_size(GTK_WINDOW (window), width_, height_);
    
    GtkWidget* vbox = gtk_vbox_new (0, 0);
    gtk_box_pack_start(GTK_BOX (vbox), canvas, 1, 1, 0);
    gtk_widget_show(canvas);    // XXX Show all causes errors!
    
    gtk_container_add(GTK_CONTAINER (window), vbox);
    gtk_widget_show(window);
    gtk_widget_show(vbox);
    
    g_signal_connect_swapped(G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
    
    isRunning_ = true;
    
    pthread_t renderThread;
    pthread_create(&renderThread, NULL, render_loop, this);
    
    gtk_main(); // Blocks as long as GTK window is open
    gdk_threads_leave();
    
    isRunning_ = false;
    
    pthread_join(renderThread, NULL);
}


int VxGtkWindowBase::onMouseEvent(vx_layer_t* layer, vx_camera_pos_t* cameraPosition, vx_mouse_event_t* event)
{
    // By default, don't consume mouse events
    return 0;
}


int VxGtkWindowBase::onKeyEvent(vx_layer_t* layer, vx_key_event_t* event)
{
    // By default, don't consume keyboard events
    return 0;
}


int handle_touch_event(vx_event_handler_t* vh, vx_layer_t* vl, vx_camera_pos_t* pos, vx_touch_event_t* mouse)
{
    // Ignore all touch events
    return 0;
}


int handle_mouse_event(vx_event_handler_t* vh, vx_layer_t* vl, vx_camera_pos_t* pos, vx_mouse_event_t* mouse)
{
    VxGtkWindowBase* window = static_cast<VxGtkWindowBase*>(vh->impl);
    return window->onMouseEvent(vl, pos, mouse);
}


int handle_key_event(vx_event_handler_t* vh, vx_layer_t* vl, vx_key_event_t* key)
{
    VxGtkWindowBase* window = static_cast<VxGtkWindowBase*>(vh->impl);
    return window->onKeyEvent(vl, key);
}


void destroy_event_handler(vx_event_handler_t* vh)
{
    // No specific state needs to be destroy
}


void* render_loop(void* data)
{
    VxGtkWindowBase* window = static_cast<VxGtkWindowBase*>(data);
    
    // Continue running until we are signaled otherwise. This happens
    // when the window is closed/Ctrl+C is received.
    
    while(window->isRunning_)
    {
        window->render();
        usleep(1000000 / window->framesPerSecond_);
    }
    
    return 0;
}

    
} // namespace eecs467
