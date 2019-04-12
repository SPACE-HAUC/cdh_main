#include <dlfcn.h>
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <OctopOS/Subscriber.h>

int main( int argc, char* argv[] ) {
   if ( argc < 2 ) {
      std::cerr << "Unable to spawn module" << std::endl;
   }
   pthread_t wait_for_subscriber_event;
   if (pthread_create( &wait_for_subscriber_event, NULL, subscriber_manager::wait_for_data, NULL )) {
      std::cerr << "Unable to start subscriber service" << std::endl;
      exit( -1 );
   }
   void* handle = dlopen( argv[0], RTLD_LAZY );

   auto octoMain = (int (*)( int, char** ))dlsym( handle, "main" );
   char** octoArgs = new char*[ argc - 1 ];
   memcpy( octoArgs, &argv[1], (argc - 1) * sizeof( char* ));

   int res = octoMain( argc - 1, octoArgs );
   pthread_kill( wait_for_subscriber_event, SIGKILL );
   dlclose( handle );
   return res;
}
