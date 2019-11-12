require 'mkmf'
$CFLAGS  += " -DRUBYINTERFACE "
create_makefile("Mars");
