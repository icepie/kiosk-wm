#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/scrnsaver.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrandr.h>


int main()
{
    Display *display = XOpenDisplay(0x0);
    if (!display)
        return 1;

    Window root = DefaultRootWindow(display);

    // Disable screen saver and screen blanking
    XSetScreenSaver(display, 0, 0, DontPreferBlanking, DefaultExposures);

    // Disable DPMS
    int event_base, error_base;
    if (DPMSQueryExtension(display, &event_base, &error_base))
    {
        DPMSDisable(display);
    }

    // This allows us to receive CreateNotify and ConfigureNotify events.
    XSelectInput(display, root, SubstructureNotifyMask);

    XFixesHideCursor(display, root);
    XSync(display, True);

    // Get screen size
    int screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);

    // Set window attributes for fullscreen
    XEvent xev;
    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    
    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.window = root;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
    xev.xclient.data.l[1] = fullscreen;
    xev.xclient.data.l[2] = 0;

    XSendEvent(display, root, False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    XFlush(display);

    // Initialize RandR extension
    int rr_event_base, rr_error_base;
    if (XRRQueryExtension(display, &rr_event_base, &rr_error_base))
    {
        // Select RandR screen change events
        XRRSelectInput(display, root, RRScreenChangeNotifyMask);
    }

    for (;;)
    {
        XEvent ev;
        XNextEvent(display, &ev);

        if (ev.type == CreateNotify || ev.type == ConfigureNotify)
        {
            if (ev.type == CreateNotify)
            {
                // MoveResize all created windows.
                XMoveResizeWindow(display, ev.xcreatewindow.window, 0, 0, screen_width, screen_height);
            }
            else if (ev.type == ConfigureNotify)
            {
                // We may also need to catch windows that move or resize themselves.
                // A lot of applications resize their windows immediately after creating them
                XConfigureEvent ce = ev.xconfigure;
                // Only MoveResize if it's not correct already.
                if (
                    ce.x != 0 ||
                    ce.y != 0 ||
                    ce.width != screen_width ||
                    ce.height != screen_height)
                {
                    XMoveResizeWindow(display, ce.window, 0, 0, screen_width, screen_height);
                }
            }
            
            XFixesHideCursor(display, root);
            XSync(display, True);
        }
        else if (ev.type == rr_event_base + RRScreenChangeNotify)
        {
            XRRScreenChangeNotifyEvent *scr_ev = (XRRScreenChangeNotifyEvent*)&ev;
            screen_width = scr_ev->width;
            screen_height = scr_ev->height;
        }

        // XForceScreenSaver(display, ScreenSaverReset);
        
        XSync(display, False);
    }

    return 0;
}
