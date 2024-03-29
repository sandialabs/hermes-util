      function pfg1lu ( first, last, ierr )
      integer  pfg1lu
c
c***********************************************************************
c     $Id$
c     
c     Copyright (2008) Sandia Corporation. Under the terms of
c     Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
c     Government retains certain rights in this software.
c     
c     Hermes is free software: you can redistribute it and/or modify
c     it under the terms of the GNU Lesser General Public License as
c     published by the Free Software Foundation, either version 3 of
c     the License, or (at your option) any later version.
c     
c     Hermes is distributed in the hope that it will be useful, but
c     WITHOUT ANY WARRANTY; without even the implied warranty of
c     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
c     GNU Lesser General Public License for more details.
c     
c     You should have received a copy of the GNU Lesser General
c     Public License along with Hermes.  If not, see
c     <http://www.gnu.org/licenses/>.
c     
C_Groups @(#)
c***********************************************************************
c
c
c ----------------------------------------------------------------------
c
c     Summary:
c
c       - This routine is a routine to find an an unused file logical
c         unit number within a user-specified range.
c
c     Input:
c       first   -  first unit # to be tested
c       last    -  last unit # to be tested
c       ierr    -  if not zero, return with no operation
c
c     Output:
c       ierr    -  error status flag;
c                      0,   No error
c                      1,   No available units in specified range
c
c     Return Value:
c       pfg1lu  =  < 0,    error has occurred, returns (-IERR)
c                  > 0,    returns unit # of file
c
c ----------------------------------------------------------------------
c
c     Internals:
c       i       -  do loop index over logical unit #s
c
c ----------------------------------------------------------------------
c     Call common blocks and associated parameter modules
c ----------------------------------------------------------------------
c
c ... Included parameter decks:
c     pfbsiz
c
c ... Included common decks:
c ... pferrs
c       r:    OFFLUN, ludbg
c
#include "pfbsiz.inc"
#include "pferrs.inc"
c
c ----------------------------------------------------------------------
c
c     Declare variables:
c
c     Passed:
      integer   first, last, ierr
c
c     Internal
      integer   i
      logical   lopen
c
c ... Parameters:
      character PFMODU*6
      parameter ( PFMODU = 'PFG1LU' )
c
c ... External Block Data
      external  pffbd
c
c ----------------------------------------------------------------------
c
c
c     write status message if DEBUG is on
c
      if (ludbg.ne.OFFLUN)then
        write (ludbg,1000) PFMODU, 'Entry', ierr
      endif
c
c     return if error flag is not zero on entry
c
      if(ierr.ne.0)then
        call pfserr ( PFMODU, 0, 0, 'Error Flag Non-zero on Entry' )
        goto 999
      endif
c
c ----------------------------------------------------------------------
c
c     search for 1st available logical unit within specified range
c
      do 1 i=first,last
        inquire ( unit=i, opened=lopen )
        if( .NOT. lopen ) then
          pfg1lu = i
          goto 999
        endif
    1 continue
c
c     no available unit numbers, return
c
      ierr   = 1
      call pfserr ( PFMODU, ierr, 0, 'No Available Logical Unit #''s' )
      pfg1lu = -1
c
c ----------------------------------------------------------------------
c
  999 continue
c
c     write status message if DEBUG is on and return
c
      if (ludbg.ne.OFFLUN)then
        write (ludbg,1000) PFMODU, 'Exit', ierr
      endif
c
      return
c
 1000 format(' %PFF-',a,'-DBG:  ',a,' Error Status = ',i5)
c
      end
