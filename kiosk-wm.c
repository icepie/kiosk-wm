#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/scrnsaver.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrandr.h>
#include <stdio.h>

int main()
{
    Display *display = XOpenDisplay(0x0);
    if (!display)
    {
        fprintf(stderr, "Unable to open X Display\n");
        return 1;
    }

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

    // Get screen size
    int screen = DefaultScreen(display);
    int screen_width = DisplayWidth(display, screen);
    int screen_height = DisplayHeight(display, screen);

    // Find the window with WM_CLASS "smart-doc-vault" and make it fullscreen
    Window target_window = None;
    Window parent, *children;
    unsigned int num_children;
    if (XQueryTree(display, root, &root, &parent, &children, &num_children))
    {
        for (unsigned int i = 0; i < num_children; i++)
        {
            XClassHint class_hint;
            if (XGetClassHint(display, children[i], &class_hint))
            {
                if (class_hint.res_class && strcmp(class_hint.res_class, "smart-doc-vault") == 0)
                {
                    target_window = children[i];
                    XFree(class_hint.res_class);
                    XFree(class_hint.res_name);
                    break;
                }
                XFree(class_hint.res_class);
                XFree(class_hint.res_name);
            }
        }
        XFree(children);
    }

    if (target_window != None)
    {
        // Set window attributes for fullscreen
        XEvent xev;
        Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
        Atom fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

        xev.xclient.type = ClientMessage;
        xev.xclient.serial = 0;
        xev.xclient.send_event = True;
        xev.xclient.window = target_window;
        xev.xclient.message_type = wm_state;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
        xev.xclient.data.l[1] = fullscreen;
        xev.xclient.data.l[2] = 0;

        XSendEvent(display, target_window, False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
        XFlush(display);
    }
    else
    {
        fprintf(stderr, "Window with WM_CLASS 'smart-doc-vault' not found\n");
    }

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

            XSync(display, True);
        }
        else if (ev.type == rr_event_base + RRScreenChangeNotify)
        {
            XRRScreenChangeNotifyEvent *scr_ev = (XRRScreenChangeNotifyEvent *)&ev;
            screen_width = scr_ev->width;
            screen_height = scr_ev->height;
        }

        XSync(display, False);
    }

    return 0;
}
