eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

# -*- perl -*-
#
# $Id$

use lib  '../../../../bin';
use PerlACE::Run_Test;

$status = 0;

$SV = new PerlACE::Process ("server", "");

print STDERR "\n\n==== Running PolicyFactory test\n";

$SV->Spawn ();

$server = $SV->WaitKill (5);

if ($server != 0) {
    print STDERR "ERROR: server returned $server\n";
    $status = 1;
}

exit $status;
