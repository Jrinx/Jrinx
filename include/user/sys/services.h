#ifndef _USER_SYS_SERVICES_H_
#define _USER_SYS_SERVICES_H_

#include <types.h>

void GET_PARTITION_STATUS(/* in */ PARTITION_STATUS_TYPE *PARTITION_STATUS,
                          /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void SET_PARTITION_MODE(/* in */ OPERATING_MODE_TYPE OPERATING_MODE,
                        /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void GET_PROCESS_ID(/* in */ PROCESS_NAME_TYPE PROCESS_NAME,
                    /* out */ PROCESS_ID_TYPE *PROCESS_ID,
                    /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void GET_PROCESS_STATUS(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                        /* out */ PROCESS_STATUS_TYPE *PROCESS_STATUS,
                        /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void CREATE_PROCESS(/* in */ PROCESS_ATTRIBUTE_TYPE *ATTRIBUTES,
                    /* out */ PROCESS_ID_TYPE *PROCESS_ID,
                    /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void SET_PRIORITY(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                  /* in */ PRIORITY_TYPE PRIORITY,
                  /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void SUSPEND_SELF(/* in */ SYSTEM_TIME_TYPE TIME_OUT,
                  /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void SUSPEND(/* in */ PROCESS_ID_TYPE PROCESS_ID,
             /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void RESUME(/* in */ PROCESS_ID_TYPE PROCESS_ID,
            /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void STOP_SELF(void);

void STOP(/* in */ PROCESS_ID_TYPE PROCESS_ID,
          /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void START(/* in */ PROCESS_ID_TYPE PROCESS_ID,
           /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void DELAYED_START(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                   /* in */ SYSTEM_TIME_TYPE DELAY_TIME,
                   /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void LOCK_PREEMPTION(/* out */ LOCK_LEVEL_TYPE *LOCK_LEVEL,
                     /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void UNLOCK_PREEMPTION(/* out */ LOCK_LEVEL_TYPE *LOCK_LEVEL,
                       /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void GET_MY_ID(/* out */ PROCESS_ID_TYPE *PROCESS_ID,
               /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void INITIALIZE_PROCESS_CORE_AFFINITY(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                                      /* in */ PROCESSOR_CORE_ID_TYPE PROCESSOR_CORE_ID,
                                      /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void GET_MY_PROCESSOR_CORE_ID(/* out */ PROCESSOR_CORE_ID_TYPE *PROCESSOR_CORE_ID,
                              /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void GET_MY_INDEX(/* out */ PROCESS_INDEX_TYPE *PROCESS_INDEX,
                  /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void TIMED_WAIT(/* in */ SYSTEM_TIME_TYPE DELAY_TIME,
                /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void PERIODIC_WAIT(/* out */ RETURN_CODE_TYPE *RETURN_CODE);

void GET_TIME(/* out */ SYSTEM_TIME_TYPE *SYSTEM_TIME,
              /* out */ RETURN_CODE_TYPE *RETURN_CODE);

void REPLENISH(/* in */ SYSTEM_TIME_TYPE BUDGET_TIME,
               /* out */ RETURN_CODE_TYPE *RETURN_CODE);

#endif
