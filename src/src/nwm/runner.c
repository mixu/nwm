#include <stdio.h>
#include "nwm.h"

void emit_callback() {

  // maybe call nwm_move_window / nwm_resize_window from here a bit..

}


int main(int argc, char **argv) {

  // add the keys to nwm
  // nwm_empty_keys();
  // nwm_add_key();
  // nwm_grab_keys();

  // set the emit function
  nwm_emit_function(emit_callback);

  // Start nwm
  int fd = nwm_init();

  // run the loop without libev
  while(true) {
    nwm_loop();
  }

  return 0;
}
