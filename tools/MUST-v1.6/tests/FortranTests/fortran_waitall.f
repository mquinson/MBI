! This file is part of MUST (Marmot Umpire Scalable Tool)
!
! Copyright (C)
!   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
!   2010-2018 Lawrence Livermore National Laboratories, United States of America
!   2013-2018 RWTH Aachen University, Federal Republic of Germany
!
! See the LICENSE file in the package base directory for details

      PROGRAM simple

CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C  @file simple_fortran.f          
C  This is a a test for the analysis of must.
C
C
C  Description:
C  Performs a send and recv.
C  This test will cause no error or warning.
C
C  @date 05.08.2011
C  @author Mathias Korepkat
C
C                                                              
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC


      IMPLICIT NONE

      INCLUDE "mpif.h"

      INTEGER ierror, my_rank, size
      INTEGER send_buf, recv_buf
      INTEGER status(MPI_STATUS_SIZE)
      INTEGER requests(4)
        data requests/4*MPI_REQUEST_NULL/

      CALL MPI_INIT(ierror)

      CALL MPI_COMM_RANK(MPI_COMM_WORLD, my_rank, ierror)
      CALL MPI_COMM_SIZE(MPI_COMM_WORLD, size, ierror)

C     Say hello
      WRITE(*,*) "Hello, I am ", my_rank, " of ", size
    
C     Check if there are enough tasks
      IF (size .LT. 2) THEN
         WRITE(*,*) "This test needs at least 2 processes!"
      ELSE

C     Send a message to rank 1
        send_buf = 1
        IF (my_rank .EQ. 0) THEN 
            CALL MPI_ISEND(send_buf, 1, MPI_INTEGER, 1, 42,
     &                 MPI_COMM_WORLD, requests(1), ierror)
        END IF

C     Recive a message from rank 0
        IF (my_rank .EQ. 1) THEN 
            CALL MPI_IRECV(recv_buf, 1, MPI_INTEGER, 0, 42,
     &                    MPI_COMM_WORLD, requests(2), ierror)
        END IF
  	CALL MPI_WAITALL(4, requests, MPI_STATUSES_IGNORE, ierror)
      END IF


C     Say bye bye
      WRITE(*,*) "Signing off, rank ", my_rank
      CALL MPI_FINALIZE(ierror)
      END
