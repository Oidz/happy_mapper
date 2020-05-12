#include <X11/Xlib.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <unistd.h>
#include <tuple>

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

std::pair<std::string, std::string>
usage(const int argc, char* const* argv);


int main(const int argc, char* const* argv) {

  std::pair<std::string, std::string> args = usage(argc, argv);

  build_window(args.first, args.second);
  
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

std::pair<std::string, std::string>
usage(const int argc, char* const* argv) {
  if(argc != 5) {
    std::cout << "Usage: " << argv[0] << " -i [image/gif] -s [sound file]\n";
    exit(1);
  }

  std::string image, sound_file;

  int c, i{0};
  while((c = getopt(argc, argv, "i:s:")) != EOF )
    {
      switch(c) {
      case ('i'):
	image = optarg;
	++i;
	break;
      case ('s'):
	sound_file = optarg;
	++i;
      break;
      default:
	break;
      }
    }

  FILE *image_check, *sound_check;
  if( !(image_check = fopen(image.c_str(), "r")) ) {
    std::cout << "Failed to open image file\n";
    exit(1);
  }
  
  if ( !(sound_check = fopen(sound_file.c_str(), "r"))) {
    std::cout << "Failed to open sound file\n";
    exit(1);
  }

  return std::make_pair(image, sound_file);
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
