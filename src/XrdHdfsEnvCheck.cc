
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define XWRITE(x, y) {if ((safe_write(x, y))) return 1; if ((safe_write("\0", 1))) return 1;}

int safe_write(const char * buf, size_t len) {
   ssize_t result = 0;
   const char * cur_buffer = buf;
   while (len) {
      result = write(1, cur_buffer, len-result);
      if (result == -1) {
         if (errno == EINTR) {
            continue;
         } else {
            return errno;
         }
      }
      cur_buffer += result;
      len -= result;
   }
   return 0;
}

#define BUFSIZE 8192
int main(int argc, char * argv[]) {

   char * classpath = getenv("CLASSPATH");
   char * libhdfs_opts = getenv("LIBHDFS_OPTS");
   char * ld_library_path = getenv("LD_LIBRARY_PATH");
   char buf[BUFSIZE+1];
   memset(buf, 0, BUFSIZE+1);
   int result;
   if (classpath && ((result = snprintf(buf, BUFSIZE, "CLASSPATH=%s", classpath)) >= 0)) {
      XWRITE(buf, result);
   }
   if (libhdfs_opts && ((result = snprintf(buf, BUFSIZE, "LIBHDFS_OPTS=%s", libhdfs_opts)) >= 0)) {
      XWRITE(buf, result);
   }
   if (ld_library_path && ((result = snprintf(buf, BUFSIZE, "LD_LIBRARY_PATH=%s", ld_library_path)) >= 0)) {
      XWRITE(buf, result)
   }

   return 0;

}

