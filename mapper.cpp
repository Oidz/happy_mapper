#include <X11/Xlib.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <string>
#include <iomanip>

/* create the x11 window overlay 
 * @param image - the image to tile on the overlay
 * @param sound_file - the sound to play on click
 */
void build_window(const std::string &image, const std::string &sound_file);

/* tile the image on the x11 window using ImageMagick
 * @param win - the window to tile the image on
 * @param image - the image to tile
 */
void tile_image(const Window &win, const std::string &image);

/* play the sound */
void sound(const std::string &sound_file);
void usage(const int argc, const char** argv);

int main(const int argc, const char** argv) {

  usage(argc, argv);
  build_window(argv[1], argv[2]);
  
  return 0;
}

void sound(const std::string &sound_file) {
  
  std::string temp {"nohup ffplay -autoexit -nodisp " + sound_file
      + " </dev/null  2>/dev/null >/dev/null &" };

  int return_code { system(temp.c_str()) };
}

void tile_image(const Window &win, const std::string &image) {
  std::string magic{ "nohup magick animate " };

  magic+=image;
  magic+= " -window " + std::to_string(win)
    + " </dev/null 2>/dev/null >/dev/null &";
  
  int return_code { system(magic.c_str()) };
}

void usage(const int argc, const char** argv) {
  if(argc != 3) {
    std::cout << "Usage: " << argv[0] << " [image/gif] [sound file]\n";
    exit(1);
  }
}

void build_window(const std::string &image, const std::string &sound_file) {

  Display *display;

  display = XOpenDisplay(NULL);
  if(display == NULL) {
    fprintf(stderr, "failed X server connection to :0");
    std::cout << "failed to open display\n";
    exit(1);
  }

  // getting basic window info:
  // get default screen of x server:
  int screen_number = DefaultScreen(display);

  // find root window id of the screen
  int root_window = RootWindow(display, screen_number);

  // set attrs in the struct
  XSetWindowAttributes x_window_attributes;
  x_window_attributes.override_redirect = True;
  x_window_attributes.do_not_propagate_mask = KeyPressMask;
  
  // OR in the values that correspond to the struct attrs
  int value_bitmask = CWDontPropagate | CWOverrideRedirect;
  
  Window win = XCreateWindow(display, RootWindow(display, screen_number), 0, 0,
		      DisplayWidth(display, screen_number),
		      DisplayHeight(display, screen_number),
		      0, CopyFromParent, CopyFromParent, CopyFromParent,
		      value_bitmask, &x_window_attributes);
 
   XSelectInput(display, win, ExposureMask | KeyPressMask | PointerMotionMask
	       | ButtonMotionMask | ButtonPressMask);

  // display will not show up until this is called
  XMapWindow(display, win);

  // don't forget to flush remaining requests to the x server
  // this broke it last time
  XFlush(display);

  tile_image(win, image);
  
  XEvent event;
  while (1) {
    XNextEvent(display, &event);

    if (event.type == ButtonPress)
      sound(sound_file);
  }

  XCloseDisplay(display);
}
