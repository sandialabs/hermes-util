      subroutine pfwfa (fid, prec, lenfa, farray, foff10, ierr)
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
c     Summary:
c
c       - This routine is a WRITE routine that writes a variable length,
c         floating array (data primitive <FARRAY>) to a PFF file.
c       - All floating point values will be converted to integers in
c         the range -32768 to 32767.  A floating point offset and
c         scaling factor are written to the file to allow reconstruction
c         of the floating point data.
c       - An arbitrary power-of-ten offset can be passed to this routine.
c         All data is assumed to be scaled by this power-of-ten.  For
c         example, if farray(1) = 1.0 and foff10 = 2, the value of
c         the 1st data value written to the file will be 100.0, etc.
c       - Maximum array length is (2**45)-1, approximately 35.2 x 10**12
c       - Note that the length of the array is integrally associated
c         with this data primitive.
c       - This operation is ONLY ALLOWED in WRITE mode !!!
c       - Format for data primitive <FARRAY>:
c           IF (Float_precision = FULL)
c             <INT>          FP_FULL
c             <INT>          foff10
c             <LONG>         LENGTH
c             <F4ARRAY>      farray
c           ELSE
c             <FLOAT>        OFFSET
c             <FLOAT>        SCALE
c             <LONG>     LENGTH
c             LOOP 1,LENGTH   \
c               <INT>         -  converted integer array
c             ENDLOOP         /
c           ENDIF
c
c ----------------------------------------------------------------------
c
c     Input:
c       fid     -  File ID of PFF file
c       prec    -  Floating-point precision type
c       farray  -  floating array to be written to PFF file
c       foff10  -  power-of-ten multiplier of farray
c       lenfa   -  length of float array to be written to PFF file
c       ierr    -  if not zero, return with no operation
c
c ----------------------------------------------------------------------
c
c     Output:
c       ierr    -  error status flag;
c                      0,  No error
c                      otherwise,  Error from called PFF utility routine
c
c ----------------------------------------------------------------------
c
c     Internals:
c       ib      -  loop index over buffers
c       ib_incr -  buffer increment for each float word processed
c       ilast   -  index of last word in buffer
c       ioff    -  offset between buffer location and full string location
c       iw      -  loop index over words in buffer
c       maxv    -  maximum value in floating point array
c       minv    -  minimum value in floating point array
c       offset  -  offset for converting integers to floats
c       pf2max  -  Maximum power-of-2 exponent that is in range for this
c                  machine's default REAL data type
c       pf2min  -  Minimum power-of-2 exponent that is in range for this
c                  machine's default REAL data type
c       pfrmin  -  Minimum floating point number that can be inverted 
c                  without floating point overflow
c       pfrmax  -  Minimum floating point number that can be represented 
c                  without floating point overflow
c       rscale  -  inverse of "scale"
c       scale   -  scale factor for converting integers to floats
c
c ----------------------------------------------------------------------
c
c     Parameters:
c       PFMODU  -  module name
c       MAXSI2  -  (2**15) - 1
c       MINSI2  -  -(2**15)
c       R216M1  -  1/(2**16-1)
c       WGTMAX  -  (2**15)/(2**16-1)
c       WGTMIN  -  (2**15-1)/(2**16-1)
c
c ----------------------------------------------------------------------
c     Call common blocks and associated parameter modules
c ----------------------------------------------------------------------
c
c ... Included parameter decks:
c     pfbsiz
c     pfstfg  -  WR, FP_FULL
c
c ... Included common decks:
c ... pfbufp
c       r:    LPRBUF
c       w:    prmbuf(*)
c ... pferrs
c       r:    OFFLUN, ludbg
c
#include "pfbsiz.inc"
#include "pfstfg.inc"
#include "pfbufp.inc"
#include "pferrs.inc"
c
c ----------------------------------------------------------------------
c
c     Declare variables:
c
c     Passed:
      integer   fid, ierr, lenfa, foff10, prec
      real      farray(*)
c
c ... Parameters:
      character PFMODU*6
      parameter ( PFMODU = 'PFWFA' )
      integer   MAXSI2, MINSI2
      real      R216M1, WGTMAX, WGTMIN
      parameter ( R216M1 = 1.0/65535.0 )
      parameter ( MAXSI2 = 32767 , MINSI2 = -32768 )
      parameter ( WGTMAX = 32768.00/65535.00 )
      parameter ( WGTMIN = 32767.00/65535.00 )
c
c     Internal
      integer   ib, ib_incr, iw, ioff, ilast, pf2max, pf2min
      real      maxv, minv, offset, pfrmax, pfrmin, rscale, scale
c
c ... External Block Data
      external  pffbd
c
c ----------------------------------------------------------------------
c
c
c ... write status message if DEBUG is on
c
      if (ludbg.ne.OFFLUN)then
        write (ludbg,1000) PFMODU, 'Entry', ierr
      endif
c
c ... return if error flag is not zero on entry
c
      if(ierr.ne.0)then
        call pfserr ( PFMODU, 0, 0, 'Error Flag Non-zero on Entry' )
        goto 999
      endif
c
c ----------------------------------------------------------------------
c
c
c ... get FLOAT LIMITS
c
      call pf_flt_limits(pf2max, pf2min, pfrmin, pfrmax)
c
      if ( prec.eq.FP_FULL ) then
c
c ..... write out "full-precision" flag
c
        call pfusio (fid, WR, 1, FP_FULL, ierr)
        call pfusio (fid, WR, 1, foff10, ierr)
        scale = 2*pfrmin
        ib_incr = 2
c
      else
c
c ..... Search data to find array offset and scale info
c
        minv = farray(1)
        maxv = farray(1)
c
        do iw=2,lenfa
          minv = min( minv, farray(iw) )
          maxv = max( maxv, farray(iw) )
        end do
c
c ..... Compute offset and scale factor
c
        offset = maxv*WGTMAX + minv*WGTMIN
        scale  = (maxv - minv)*R216M1
c
c ..... Compute inverse of scale factor (if nonzero)
c
        if ( scale.gt.pfrmin ) then
          rscale = 1./scale
        endif
c
c ..... Write out array offset and scale info
c
        call pfuf2i ( offset, foff10, prmbuf, ierr )
        call pfuf2i ( scale, foff10, prmbuf(4), ierr )
        call pfusio (fid, WR, 6, prmbuf, ierr)
c
        ib_incr = 1
      endif
c
c ... write length to file as a <LONG>
c
      call pful2i ( lenfa, prmbuf, ierr )
      call pfusio (fid, WR, 3, prmbuf, ierr)
      if ( ierr.ne.0 ) goto 999
c
c ... compute integer array from floating array using offset and scale
c ... factor; then write array to file (1 buffer at a time)
c
      do ib=1,lenfa,LPRBUF/ib_incr
c
        ioff = ib - 1
        ilast = min(lenfa,ioff+LPRBUF/ib_incr)
c
        if ( scale.gt.pfrmin ) then
c
          if ( prec.eq.FP_FULL ) then
c
            call pfmf4 (WR, ilast-ioff, prmbuf, farray(ib), ierr)
c
          else
c
c ......... if scale .NE. 0.0, need to calculate each integer
c
            do iw=ib,ilast
              prmbuf(iw - ioff) = min ( MAXSI2 , max ( MINSI2 ,
     &                            nint(rscale*(farray(iw) - offset)) ))
            end do
          endif
c
        else if (ib.eq.1) then
c
c ....... if scale = 0.0, need to send all zeros; but only need to fill
c ....... the buffer once (the 1st time)
c
          do iw=1,ilast
            prmbuf(iw) = 0
          end do
c
        endif
c
        call pfusio (fid, WR, ib_incr*(ilast-ioff), prmbuf, ierr)
        if ( ierr.ne.0 ) goto 999
c
      end do
c
c ----------------------------------------------------------------------
c
  999 continue
c
c ... write status message if DEBUG is on and return
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
