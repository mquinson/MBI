! This file is part of MUST (Marmot Umpire Scalable Tool)
!
! Copyright (C)
!   2010-2016 ZIH, Technische Universitaet Dresden, Federal Republic of Germany
!   2010-2018 Lawrence Livermore National Laboratories, United States of America
!   2013-2018 RWTH Aachen University, Federal Republic of Germany
!
! See the LICENSE file in the package base directory for details

      PROGRAM datatype

CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C  @file datatype_fortran.f          
C  This is a a test for the analysis of must.
C
C
C  Description:
C  Creates a MPI datatype, performs a send and irecv and frees the datatype.
C  this test will cause no error or warning.
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
      INTEGER request
      INTEGER newType
      CALL MPI_INIT(ierror)

      CALL MPI_COMM_RANK(MPI_COMM_WORLD, my_rank, ierror)
      CALL MPI_COMM_SIZE(MPI_COMM_WORLD, size, ierror)

C     Say hello
      WRITE(*,*) "Hello, I am ", my_rank, " of ", size
    
C     Check if there are enough tasks
      IF (size .LT. 2) THEN
         WRITE(*,*) "This test needs at least 2 processes!"
      ELSE

C     Create a derived datatype
      CALL MPI_TYPE_CONTIGUOUS(1,MPI_INTEGER,newType, ierror);
      CALL MPI_TYPE_COMMIT(newType,ierror);

C     Send a message to rank 1
        send_buf = 1
        IF (my_rank .EQ. 0) THEN 
            CALL MPI_SEND(send_buf, 1, newType, 1, 42,
     &                 MPI_COMM_WORLD, ierror)
        END IF

C     Recive a message from rank 0
        IF (my_rank .EQ. 1) THEN 
            CALL MPI_IRECV(recv_buf, 1, newType, 0, 42,
     &                    MPI_COMM_WORLD, request, ierror)

            CALL MPI_WAIT(request,status,ierror)
        END IF
      END IF
      
C     Free new datatype
      CALL MPI_TYPE_FREE(newType,ierror)

C     Say bye bye
      WRITE(*,*) "Signing off, rank ", my_rank
      CALL MPI_FINALIZE(ierror)
      END
