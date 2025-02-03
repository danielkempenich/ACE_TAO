#include "ace/FILE_IO.h"

#include "ace/Log_Category.h"
#include "ace/OS_NS_sys_stat.h"
#include "ace/OS_Memory.h"
#include "ace/Truncate.h"
#if defined (ACE_HAS_ALLOC_HOOKS)
# include "ace/Malloc_Base.h"
#endif /* ACE_HAS_ALLOC_HOOKS */

#if !defined (__ACE_INLINE__)
#include "ace/FILE_IO.inl"
#endif /* __ACE_INLINE__ */

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

ACE_ALLOC_HOOK_DEFINE(ACE_FILE_IO)

void
ACE_FILE_IO::dump () const
{
#if defined (ACE_HAS_DUMP)
  ACE_TRACE ("ACE_FILE_IO::dump");

  ACELIB_DEBUG ((LM_DEBUG, ACE_BEGIN_DUMP, this));
  this->addr_.dump ();
  ACELIB_DEBUG ((LM_DEBUG, ACE_END_DUMP));
#endif /* ACE_HAS_DUMP */
}

// Simple-minded do nothing constructor.

ACE_FILE_IO::ACE_FILE_IO ()
{
  ACE_TRACE ("ACE_FILE_IO::ACE_FILE_IO");
}

// Send N char *ptrs and int lengths.  Note that the char *'s precede
// the ints (basically, an varargs version of writev).  The count N is
// the *total* number of trailing arguments, *not* a couple of the
// number of tuple pairs!

ssize_t
ACE_FILE_IO::send (size_t n, ...) const
{
  ACE_TRACE ("ACE_FILE_IO::send");
#ifdef ACE_LACKS_VA_FUNCTIONS
  ACE_UNUSED_ARG (n);
  ACE_NOTSUP_RETURN (-1);
#else
  va_list argp;
  int total_tuples = ACE_Utils::truncate_cast<int> (n / 2);
  iovec *iovp = 0;
#if defined (ACE_HAS_ALLOCA)
  iovp = (iovec *) alloca (total_tuples * sizeof (iovec));
#else
# ifdef ACE_HAS_ALLOC_HOOKS
  ACE_ALLOCATOR_RETURN (iovp, (iovec *)
                        ACE_Allocator::instance ()->malloc (total_tuples *
                                                            sizeof (iovec)),
                        -1);
# else
  ACE_NEW_RETURN (iovp,
                  iovec[total_tuples],
                  -1);
# endif /* ACE_HAS_ALLOC_HOOKS */
#endif /* !defined (ACE_HAS_ALLOCA) */

  va_start (argp, n);

  for (int i = 0; i < total_tuples; i++)
    {
      iovp[i].iov_base = va_arg (argp, char *);
      iovp[i].iov_len  = va_arg (argp, int);
    }

  ssize_t result = ACE_OS::writev (this->get_handle (),
                                   iovp,
                                   total_tuples);
#if !defined (ACE_HAS_ALLOCA)
# ifdef ACE_HAS_ALLOC_HOOKS
  ACE_Allocator::instance ()->free (iovp);
# else
  delete [] iovp;
# endif /* ACE_HAS_ALLOC_HOOKS */
#endif /* !defined (ACE_HAS_ALLOCA) */
  va_end (argp);
  return result;
#endif // ACE_LACKS_VA_FUNCTIONS
}

// This is basically an interface to ACE_OS::readv, that doesn't use
// the struct iovec explicitly.  The ... can be passed as an arbitrary
// number of (char *ptr, int len) tuples.  However, the count N is the
// *total* number of trailing arguments, *not* a couple of the number
// of tuple pairs!

ssize_t
ACE_FILE_IO::recv (size_t n, ...) const
{
  ACE_TRACE ("ACE_FILE_IO::recv");
#ifdef ACE_LACKS_VA_FUNCTIONS
  ACE_UNUSED_ARG (n);
  ACE_NOTSUP_RETURN (-1);
#else
  va_list argp;
  int total_tuples = ACE_Utils::truncate_cast<int> (n / 2);
  iovec *iovp = 0;
#if defined (ACE_HAS_ALLOCA)
  iovp = (iovec *) alloca (total_tuples * sizeof (iovec));
#else
# ifdef ACE_HAS_ALLOC_HOOKS
  ACE_ALLOCATOR_RETURN (iovp, (iovec *)
                        ACE_Allocator::instance ()->malloc (total_tuples *
                                                            sizeof (iovec)),
                        -1);
# else
  ACE_NEW_RETURN (iovp,
                  iovec[total_tuples],
                  -1);
# endif /* ACE_HAS_ALLOC_HOOKS */
#endif /* !defined (ACE_HAS_ALLOCA) */

  va_start (argp, n);

  for (int i = 0; i < total_tuples; i++)
    {
      iovp[i].iov_base = va_arg (argp, char *);
      iovp[i].iov_len  = va_arg (argp, int);
    }

  ssize_t const result = ACE_OS::readv (this->get_handle (),
                                        iovp,
                                        total_tuples);
#if !defined (ACE_HAS_ALLOCA)
# ifdef ACE_HAS_ALLOC_HOOKS
  ACE_Allocator::instance ()->free (iovp);
# else
  delete [] iovp;
# endif /* ACE_HAS_ALLOC_HOOKS */
#endif /* !defined (ACE_HAS_ALLOCA) */
  va_end (argp);
  return result;
#endif // ACE_LACKS_VA_FUNCTIONS
}

// Allows a client to read from a file without having to provide a
// buffer to read.  This method determines how much data is in the
// file, allocates a buffer of this size, reads in the data, and
// returns the number of bytes read.

ssize_t
ACE_FILE_IO::recvv (iovec *io_vec)
{
  ACE_TRACE ("ACE_FILE_IO::recvv");

  io_vec->iov_base = 0;
  ACE_OFF_T const length = ACE_OS::filesize (this->get_handle ());

  if (length > 0)
    {
      // Restrict to max size we can record in iov_len.
      size_t len = ACE_Utils::truncate_cast<u_long> (length);
#if defined (ACE_HAS_ALLOC_HOOKS)
      ACE_ALLOCATOR_RETURN (io_vec->iov_base,
                            static_cast<char*>(ACE_Allocator::instance()->malloc(sizeof(char) * len)),
                            -1);
#else
      ACE_NEW_RETURN (io_vec->iov_base,
                      char[len],
                      -1);
#endif /* ACE_HAS_ALLOC_HOOKS */
      io_vec->iov_len = static_cast<u_long> (this->recv_n (io_vec->iov_base,
                                                           len));
      return io_vec->iov_len;
    }
  else
    {
      return ACE_Utils::truncate_cast<ssize_t> (length);
    }
}

ACE_END_VERSIONED_NAMESPACE_DECL
