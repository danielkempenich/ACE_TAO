// $Id:$

#include "TestS.h"
#include "ace/Get_Opt.h"
#include "orbsvcs/FaultTolerance/FT_ClientService_Activate.h"
#include "Hello.h"

ACE_RCSID(Hello, client, "$Id:$")

const char *ior = "file://ior.ior";
const char *iogr = "file://iogr.ior";

int
parse_args (int argc, char *argv[])
{
  ACE_Get_Opt get_opts (argc, argv, "k:l:");
  int c;

  while ((c = get_opts ()) != -1)
    switch (c)
      {
      case 'k':
        ior = get_opts.opt_arg ();
        break;
      case 'l':
        iogr = get_opts.opt_arg ();
        break;
      case '?':
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s "
                           "-k <ior> "
                           "\n",
                           argv [0]),
                          -1);
      }
  // Indicates sucessful parsing of the command line
  return 0;
}

int
main (int argc, char *argv[])
{
  CORBA::Boolean result = 0;
  ACE_TRY_NEW_ENV
    {
      CORBA::ORB_var orb =
        CORBA::ORB_init (argc, argv, "" ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      if (parse_args (argc, argv) != 0)
        return 1;

      // First perform the test with an IOR
      CORBA::Object_var tmp =
        orb->string_to_object(ior ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      Test::Hello_var hello =
        Test::Hello::_narrow(tmp.in () ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      if (CORBA::is_nil (hello.in ()))
        {
          ACE_ERROR_RETURN ((LM_DEBUG,
                             "Test failed - Not regression - Unexpected Nil Test::Hello reference <%s>\n",
                             ior),
                            1);
        }

      // Check this isn't generating exceptions for any other reason
      hello->ping (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;

      if (hello->has_ft_request_service_context (ACE_ENV_SINGLE_ARG_PARAMETER))
        {
          ACE_DEBUG ((LM_ERROR, "ERROR - REGRESSION - Request made on a plain IOR has a FT_REQUEST service context.\n" ));
          result = 1;
        }
      else
        {
          ACE_DEBUG ((LM_DEBUG, "Request made on a plain IOR has no FT_REQUEST service context. This is OK.\n" ));
        }

      // Now repeat the test (for the converse result) with an IOGR
      tmp =
        orb->string_to_object(iogr ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      hello =
        Test::Hello::_narrow(tmp.in () ACE_ENV_ARG_PARAMETER);
      ACE_TRY_CHECK;

      if (CORBA::is_nil (hello.in ()))
        {
          ACE_ERROR_RETURN ((LM_DEBUG,
                             "Test failed - Not regression - Unexpected Nil Test::Hello reference <%s>\n",
                             iogr),
                            1);
        }

      // Check this isn't generating transients for any other reason
      hello->ping (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;

      if (! hello->has_ft_request_service_context (ACE_ENV_SINGLE_ARG_PARAMETER))
        {
          ACE_DEBUG ((LM_ERROR, "ERROR - REGRESSION - Request made on an IOGR has no FT_REQUEST service context.\n" ));
          result = 1;
        }
      else
        {
          ACE_DEBUG ((LM_DEBUG, "Request made on an IOGR has a FT_REQUEST service context. This is OK.\n" ));
        }

      hello->shutdown (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;

      orb->destroy (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION,
                           "Test failed (Not regression) because unexpected exception caught:");
      return 1;
    }
  ACE_ENDTRY;

  if (result)
    {
      ACE_DEBUG ((LM_ERROR, "Error: REGRESSION identified!!!\n"));
    }
  else
    {
      ACE_DEBUG ((LM_DEBUG, "Test passed !!!\n"));
    }
  return result;
}
