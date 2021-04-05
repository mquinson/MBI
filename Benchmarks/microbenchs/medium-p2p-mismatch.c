////////////////// MPI bugs collection header //////////////////
//
// Origin: ISP (http://formalverification.cs.utah.edu/ISP_Tests/)
//
// Description: A lot of point to point communication with ops type mismatch
//
//// List of features
// P2P: Incorrect
// iP2P: Incorrect
// PERS: Lacking
// COLL: Correct
// iCOLL: Lacking
// TOPO: Lacking
// IO: Lacking
// RMA: Lacking
// PROB: Lacking
// COM: Lacking
// GRP: Lacking
// DATA: Lacking
// OP: Lacking
//
//// List of errors
// deadlock: never
// numstab: transient
// segfault: never
// mpierr: never
// resleak: never
// livelock: never
// datarace: never
//
// Test: mpirun -np 2 ${EXE}
// Expect: numstab
//
////////////////// End of MPI bugs collection header //////////////////
//////////////////       original file begins        //////////////////

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <string.h>

#ifndef MPI_MAX_PROCESSOR_NAME
#define MPI_MAX_PROCESSOR_NAME 1024
#endif

#define BUF_SIZE 128
#define NUM_SEND_TYPES 8
#define NUM_PERSISTENT_SEND_TYPES 4
#define NUM_BSEND_TYPES 2
#define NUM_COMPLETION_MECHANISMS 8
#define NUM_RECV_TYPES 2

int main(int argc, char **argv) {
  int nprocs = -1;
  int rank = -1;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen = 128;
  int bbuf[(BUF_SIZE + MPI_BSEND_OVERHEAD) * 2 * NUM_BSEND_TYPES];
  int buf[BUF_SIZE * 2 * NUM_SEND_TYPES];
  int i, j, k, l, m, at_size, send_t_number, index, outcount, total, flag;
  int num_errors, error_count, indices[2 * NUM_SEND_TYPES];
  MPI_Request aReq[2 * NUM_SEND_TYPES];
  MPI_Status aStatus[2 * NUM_SEND_TYPES];

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  printf("rank %d is alive on %s\n", rank, processor_name);

  if (nprocs < 2) {
    printf("\033[0;31m! This test requires at least 2 processes !\033[0;0m\n");
    MPI_Finalize();
    return 0;
  }

  MPI_Buffer_attach(bbuf, sizeof(int) * (BUF_SIZE + MPI_BSEND_OVERHEAD) * 2 *
                              NUM_BSEND_TYPES);

  if (rank == 0) {
    /* set up persistent sends... */
    send_t_number = NUM_SEND_TYPES - NUM_PERSISTENT_SEND_TYPES;

    MPI_Send_init(&buf[send_t_number * 2 * BUF_SIZE], BUF_SIZE, MPI_INT, 1,
                  send_t_number * 2, MPI_COMM_WORLD, &aReq[send_t_number * 2]);
    MPI_Send_init(&buf[(send_t_number * 2 + 1) * BUF_SIZE], BUF_SIZE, MPI_INT,
                  1, send_t_number * 2 + 1, MPI_COMM_WORLD,
                  &aReq[send_t_number * 2 + 1]);

    send_t_number++;

    MPI_Bsend_init(&buf[send_t_number * 2 * BUF_SIZE], BUF_SIZE, MPI_INT, 1,
                   send_t_number * 2, MPI_COMM_WORLD, &aReq[send_t_number * 2]);
    MPI_Bsend_init(&buf[(send_t_number * 2 + 1) * BUF_SIZE], BUF_SIZE, MPI_INT,
                   1, send_t_number * 2 + 1, MPI_COMM_WORLD,
                   &aReq[send_t_number * 2 + 1]);

    send_t_number++;

    MPI_Rsend_init(&buf[send_t_number * 2 * BUF_SIZE], BUF_SIZE, MPI_INT, 1,
                   send_t_number * 2, MPI_COMM_WORLD, &aReq[send_t_number * 2]);
    MPI_Rsend_init(&buf[(send_t_number * 2 + 1) * BUF_SIZE], BUF_SIZE, MPI_INT,
                   1, send_t_number * 2 + 1, MPI_COMM_WORLD,
                   &aReq[send_t_number * 2 + 1]);

    send_t_number++;

    MPI_Ssend_init(&buf[send_t_number * 2 * BUF_SIZE], BUF_SIZE, MPI_INT, 1,
                   send_t_number * 2, MPI_COMM_WORLD, &aReq[send_t_number * 2]);
    MPI_Ssend_init(&buf[(send_t_number * 2 + 1) * BUF_SIZE], BUF_SIZE, MPI_INT,
                   1, send_t_number * 2 + 1, MPI_COMM_WORLD,
                   &aReq[send_t_number * 2 + 1]);
  }

  for (m = 0; m < NUM_RECV_TYPES; m++) {
    if ((m == 1) && (rank == 1)) {
      /* set up the persistent receives... */
      for (j = 0; j < NUM_SEND_TYPES * 2; j += 2) {
        MPI_Recv_init(&buf[j * BUF_SIZE], BUF_SIZE, MPI_INT, 0, j,
                      MPI_COMM_WORLD, &aReq[j]);
        MPI_Recv_init(&buf[(j + 1) * BUF_SIZE], BUF_SIZE * sizeof(int),
                      MPI_BYTE, 0, j + 1, MPI_COMM_WORLD, &aReq[j + 1]);
      }
    }

    for (l = 0; l < (NUM_COMPLETION_MECHANISMS * 2); l++) {
      for (k = 0; k < (NUM_COMPLETION_MECHANISMS * 2); k++) {
        if (rank == 0) {
          /* initialize all of the send buffers */
          for (j = 0; j < NUM_SEND_TYPES; j++) {
            for (i = 0; i < BUF_SIZE; i++) {
              buf[2 * j * BUF_SIZE + i] = i;
              buf[((2 * j + 1) * BUF_SIZE) + i] = BUF_SIZE - 1 - i;
            }
          }
        } else if (rank == 1) {
          /* zero out all of the receive buffers */
          bzero(buf, sizeof(int) * BUF_SIZE * 2 * NUM_SEND_TYPES);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        if (rank == 0) {
          /* set up transient sends... */
          send_t_number = 0;

          MPI_Isend(&buf[send_t_number * 2 * BUF_SIZE], BUF_SIZE, MPI_INT, 1,
                    send_t_number * 2, MPI_COMM_WORLD,
                    &aReq[send_t_number * 2]);
          MPI_Isend(&buf[(send_t_number * 2 + 1) * BUF_SIZE], BUF_SIZE, MPI_INT,
                    1, send_t_number * 2 + 1, MPI_COMM_WORLD,
                    &aReq[send_t_number * 2 + 1]);

          send_t_number++;

          MPI_Ibsend(&buf[send_t_number * 2 * BUF_SIZE], BUF_SIZE, MPI_INT, 1,
                     send_t_number * 2, MPI_COMM_WORLD,
                     &aReq[send_t_number * 2]);
          MPI_Ibsend(&buf[(send_t_number * 2 + 1) * BUF_SIZE], BUF_SIZE,
                     MPI_INT, 1, send_t_number * 2 + 1, MPI_COMM_WORLD,
                     &aReq[send_t_number * 2 + 1]);

          send_t_number++;

          /* Barrier to ensure receives are posted for rsends... */
          MPI_Barrier(MPI_COMM_WORLD);

          MPI_Irsend(&buf[send_t_number * 2 * BUF_SIZE], BUF_SIZE, MPI_INT, 1,
                     send_t_number * 2, MPI_COMM_WORLD,
                     &aReq[send_t_number * 2]);
          MPI_Irsend(&buf[(send_t_number * 2 + 1) * BUF_SIZE], BUF_SIZE,
                     MPI_INT, 1, send_t_number * 2 + 1, MPI_COMM_WORLD,
                     &aReq[send_t_number * 2 + 1]);

          send_t_number++;

          MPI_Issend(&buf[send_t_number * 2 * BUF_SIZE], BUF_SIZE, MPI_INT, 1,
                     send_t_number * 2, MPI_COMM_WORLD,
                     &aReq[send_t_number * 2]);
          MPI_Issend(&buf[(send_t_number * 2 + 1) * BUF_SIZE], BUF_SIZE,
                     MPI_INT, 1, send_t_number * 2 + 1, MPI_COMM_WORLD,
                     &aReq[send_t_number * 2 + 1]);

          /* just to be paranoid */
          send_t_number++;
          assert(send_t_number == NUM_SEND_TYPES - NUM_PERSISTENT_SEND_TYPES);

          /* start the persistent sends... */
          if (k % 2) {
            MPI_Startall(NUM_PERSISTENT_SEND_TYPES * 2,
                         &aReq[2 * send_t_number]);
          } else {
            for (j = 0; j < NUM_PERSISTENT_SEND_TYPES * 2; j++) {
              MPI_Start(&aReq[2 * send_t_number + j]);
            }
          }

          /* complete the sends */
          switch (k / 2) {
          case 0:
            /* use MPI_Wait */
            for (j = 0; j < NUM_SEND_TYPES * 2; j++) {
              MPI_Wait(&aReq[j], &aStatus[j]);
            }
            break;
          case 1:
            /* use MPI_Waitall */
            MPI_Waitall(NUM_SEND_TYPES * 2, aReq, aStatus);
            break;
          case 2:
            /* use MPI_Waitany */
            for (j = 0; j < NUM_SEND_TYPES * 2; j++) {
              MPI_Waitany(NUM_SEND_TYPES * 2, aReq, &index, aStatus);
            }
            break;
          case 3:
            /* use MPI_Waitsome */
            total = 0;
            while (total < NUM_SEND_TYPES * 2) {
              MPI_Waitsome(NUM_SEND_TYPES * 2, aReq, &outcount, indices,
                           aStatus);

              total += outcount;
            }
            break;
          case 4:
            /* use MPI_Test */
            for (j = 0; j < NUM_SEND_TYPES * 2; j++) {
              flag = 0;

              while (!flag) {
                MPI_Test(&aReq[j], &flag, &aStatus[j]);
              }
            }
            break;
          case 5:
            /* use MPI_Testall */
            flag = 0;
            while (!flag) {
              MPI_Testall(NUM_SEND_TYPES * 2, aReq, &flag, aStatus);
            }
            break;
          case 6:
            /* use MPI_Testany */
            for (j = 0; j < NUM_SEND_TYPES * 2; j++) {
              flag = 0;
              while (!flag) {
                MPI_Testany(NUM_SEND_TYPES * 2, aReq, &index, &flag, aStatus);
              }
            }
            break;
          case 7:
            /* use MPI_Testsome */
            total = 0;
            while (total < NUM_SEND_TYPES * 2) {
              outcount = 0;

              while (!outcount) {
                MPI_Testsome(NUM_SEND_TYPES * 2, aReq, &outcount, indices,
                             aStatus);
              }

              total += outcount;
            }
            break;
          default:
            assert(0);
            break;
          }
        } else if (rank == 1) {
          /* start receives for all of the sends */
          if (m == 0) {
            for (j = 0; j < NUM_SEND_TYPES * 2; j += 2) {
              MPI_Irecv(&buf[j * BUF_SIZE], BUF_SIZE, MPI_INT, 0, j,
                        MPI_COMM_WORLD, &aReq[j]);
              MPI_Irecv(&buf[(j + 1) * BUF_SIZE], BUF_SIZE * sizeof(int),
                        MPI_BYTE, 0, j + 1, MPI_COMM_WORLD, &aReq[j + 1]);
            }
          } else {
            /* start the persistent receives... */
            if (l % 2) {
              MPI_Startall(NUM_SEND_TYPES * 2, aReq);
            } else {
              for (j = 0; j < NUM_SEND_TYPES * 2; j++) {
                MPI_Start(&aReq[j]);
              }
            }
          }

          /* Barrier to ensure receives are posted for rsends... */
          MPI_Barrier(MPI_COMM_WORLD);

          /* complete all of the receives... */
          switch (l / 2) {
          case 0:
            /* use MPI_Wait */
            for (j = 0; j < NUM_SEND_TYPES * 2; j++) {
              MPI_Wait(&aReq[j], &aStatus[j]);
            }
            break;
          case 1:
            /* use MPI_Waitall */
            MPI_Waitall(NUM_SEND_TYPES * 2, aReq, aStatus);
            break;
          case 2:
            /* use MPI_Waitany */
            for (j = 0; j < NUM_SEND_TYPES * 2; j++) {
              MPI_Waitany(NUM_SEND_TYPES * 2, aReq, &index, aStatus);
            }
            break;
          case 3:
            /* use MPI_Waitsome */
            total = 0;
            while (total < NUM_SEND_TYPES * 2) {
              MPI_Waitsome(NUM_SEND_TYPES * 2, aReq, &outcount, indices,
                           aStatus);

              total += outcount;
            }
            break;
          case 4:
            /* use MPI_Test */
            for (j = 0; j < NUM_SEND_TYPES * 2; j++) {
              flag = 0;

              while (!flag) {
                MPI_Test(&aReq[j], &flag, &aStatus[j]);
              }
            }
            break;
          case 5:
            /* use MPI_Testall */
            flag = 0;
            while (!flag) {
              MPI_Testall(NUM_SEND_TYPES * 2, aReq, &flag, aStatus);
            }
            break;
          case 6:
            /* use MPI_Testany */
            for (j = 0; j < NUM_SEND_TYPES * 2; j++) {
              flag = 0;
              while (!flag) {
                MPI_Testany(NUM_SEND_TYPES * 2, aReq, &index, &flag, aStatus);
              }
            }
            break;
          case 7:
            /* use MPI_Testsome */
            total = 0;
            while (total < NUM_SEND_TYPES * 2) {
              outcount = 0;

              while (!outcount) {
                MPI_Testsome(NUM_SEND_TYPES * 2, aReq, &outcount, indices,
                             aStatus);
              }
              total += outcount;
            }
            break;
          default:
            assert(0);
            break;
          }
        } else {
          /* Barrier to ensure receives are posted for rsends... */
          MPI_Barrier(MPI_COMM_WORLD);
        }
      }
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  if (rank == 0) {
    /* free the persistent send requests */
    for (i = 2 * (NUM_SEND_TYPES - NUM_PERSISTENT_SEND_TYPES);
         i < 2 * NUM_SEND_TYPES; i++) {
      MPI_Request_free(&aReq[i]);
    }
  } else if (rank == 1) {
    /* free the persistent receive requests */
    for (i = 0; i < 2 * NUM_SEND_TYPES; i++) {
      MPI_Request_free(&aReq[i]);
    }
  }

  MPI_Buffer_detach(bbuf, &at_size);

  assert(at_size ==
         sizeof(int) * (BUF_SIZE + MPI_BSEND_OVERHEAD) * 2 * NUM_BSEND_TYPES);

  MPI_Finalize();
  printf("\033[0;32mrank %d Finished normally\033[0;0m\n", rank);
  return 0;
}
