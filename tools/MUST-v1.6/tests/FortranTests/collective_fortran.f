! This file is part of MUST (Marmot Umpire Scalable Tool)
!
! Copyright (C)
!   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
!   2010-2018 Lawrence Livermore National Laboratories, United States of America
!   2013-2018 RWTH Aachen University, Federal Republic of Germany
!
! See the LICENSE file in the package base directory for details

      PROGRAM collective

CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C  @file collective_fortran.f          
C  This is a a test for the analysis of must.
C
C
C  Description:
C  Performs a call to mpi_reduce.
C  This test will cause no error or warning.
C
C  @date 08.08.2011
C  @author Mathias Korepkat
C
C                                                              
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC


      IMPLICIT NONE

      INCLUDE "mpif.h"

      INTEGER ierror, my_rank, size
      INTEGER send_buf, recv_buf
      INTEGER status(MPI_STATUS_SIZE)

      CALL MPI_INIT(ierror)

      CALL MPI_COMM_RANK(MPI_COMM_WORLD, my_rank, ierror)
      CALL MPI_COMM_SIZE(MPI_COMM_WORLD, size, ierror)

C     Say hello
      WRITE(*,*) "Hello, I am ", my_rank, " of ", size
    
C     Check if there are enough tasks
      IF (size .LT. 2) THEN
         WRITE(*,*) "This test needs at least 2 processes!"
      ELSE

C       reduce the number of send_buf by the SUM operation
        send_buf = 1
        CALL MPI_REDUCE(send_buf,recv_buf,1,MPI_INTEGER,MPI_SUM,
     &                  0,MPI_COMM_WORLD,ierror)
      
      END IF

C     Say bye bye
      WRITE(*,*) "Signing off, rank ", my_rank
      CALL MPI_FINALIZE(ierror)
      END
