#include <eecs467/vx_gtk_window_base.hpp>
#include <vx/vxo_text.h>
#include <vx/vxo_pix_coords.h>
#include <vx/vx.h>
#include <iostream>


class VxWindow : public eecs467::VxGtkWindowBase
{
public:
    
    VxWindow(int argc, char** argv)
    : VxGtkWindowBase(argc, argv, 640, 480, 15)
    , frameCount_(0)
    {
    }
    
    virtual int onMouseEvent(vx_layer_t* layer, vx_camera_pos_t* cameraPosition, vx_mouse_event_t* event)
    {
        std::cout << "Mouse event at (" << event->x << ',' << event->y << ") Mask:" << event->button_mask << '\n';
        return 0;
    }
    
    virtual int onKeyEvent(vx_layer_t* layer, vx_key_event_t* event)
    {
        std::cout << "Key press: Released:" << event->released << " Key:" << event->key_code << '\n';
        return 0;
    }
    
private:
    
    int frameCount_;
    
    virtual void render(void)
    {
        std::cout << "Rendering frame number: " << ++frameCount_ << '\n';
        
        // Draw some simple text in the center of the screen.
        vx_buffer_t* helloBuf = vx_world_get_buffer(world_, "hello");
        vx_object_t* helloText = vxo_text_create(VXO_TEXT_ANCHOR_CENTER, "Hello World!");
        vx_buffer_add_back(helloBuf, vxo_pix_coords(VX_ORIGIN_CENTER, helloText));
        vx_buffer_swap(helloBuf);
    }
};


int main(int argc, char** argv)
{
    VxWindow window(argc, argv);
    window.run();
    
    std::cout << "Window closed. Exiting program...\n";
    
    return 0;
}
