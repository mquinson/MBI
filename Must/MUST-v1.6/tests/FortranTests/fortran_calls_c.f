! This file is part of MUST (Marmot Umpire Scalable Tool)
!
! Copyright (C)
!   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
!   2010-2018 Lawrence Livermore National Laboratories, United States of America
!   2013-2018 RWTH Aachen University, Federal Republic of Germany
!
! See the LICENSE file in the package base directory for details

      PROGRAM fortranc

CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C  @file fortranc.f          
C  This is a a test for the analysis of must.
C
C
C  Description:
C  Performs a MPI_Send and calls recv, what is a C function that calls a MPI_Recv.
C
C
C  @date 16.08.2011
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

C     Send a message to rank 1
      send_buf = 1
      IF (my_rank .EQ. 0) THEN 
          CALL MPI_SEND(send_buf, 1, MPI_INTEGER, 1, 42,
     &                  MPI_COMM_WORLD, ierror)
          WRITE(*,*) "Rank 0 sends: ",send_buf
      END IF

C     Recive a message from rank 0
        IF (my_rank .EQ. 1) THEN 
            recv_buf = -1
            CALL myrecv(recv_buf)
            WRITE(*,*) "Rank 1 recives: ",recv_buf
        END IF
      END IF


C     Say bye bye
      WRITE(*,*) "Signing off, rank ", my_rank
      CALL MPI_FINALIZE(ierror)
      END
