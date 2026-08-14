# Multithreaded-SIMD-Raytracer

Accompanying blog post:

Under construction

## Build & run instructions

`./build.sh -release` on a linux machine with an AVX2 and FMA compatible processor.
`cd build`
`./main`

This will output an `image.ppm` containing the image. 

## Current Profile

Reference profile - [Peter Shirley's excellent inOneWeekend resource](https://raytracing.github.io/books/RayTracingInOneWeekend.html):

```
 Performance counter stats for './inOneWeekend':

                 0      context-switches:u               #      0.0 cs/sec  cs_per_second     
                 0      cpu-migrations:u                 #      0.0 migrations/sec  migrations_per_second
               155      page-faults:u                    #      2.8 faults/sec  page_faults_per_second
         56,272.65 msec task-clock:u                     #      1.0 CPUs  CPUs_utilized       
    25,577,038,181      L1-dcache-load-misses:u          #     10.3 %  l1d_miss_rate            (30.00%)
           641,769      L1-icache-load-misses:u          #      0.1 %  l1i_miss_rate            (30.00%)
        79,315,319      branch-misses:u                  #      0.1 %  branch_miss_rate         (30.00%)
    65,176,132,378      branches:u                       #   1158.2 M/sec  branch_frequency     (30.00%)
   229,604,451,070      cpu-cycles:u                     #      4.1 GHz  cycles_frequency       (40.00%)
   617,819,893,396      instructions:u                   #      2.7 instructions  insn_per_cycle  (40.00%)
     1,023,007,001      stalled-cycles-frontend:u        #     0.00 frontend_cycles_idle        (40.00%)
         4,179,549      dTLB-loads:u                     #      5.4 %  dtlb_miss_rate           (30.00%)
            20,082      iTLB-loads:u                     #     31.1 %  itlb_miss_rate           (30.00%)

      56.281917514 seconds time elapsed

      55.656189000 seconds user
       0.007932000 seconds sys
```

My initial implementation:

```
 Performance counter stats for './inOneWeekend':

                 0      context-switches:u               #      0.0 cs/sec  cs_per_second     
                 0      cpu-migrations:u                 #      0.0 migrations/sec  migrations_per_second
               155      page-faults:u                    #      2.8 faults/sec  page_faults_per_second
         56,272.65 msec task-clock:u                     #      1.0 CPUs  CPUs_utilized       
    25,577,038,181      L1-dcache-load-misses:u          #     10.3 %  l1d_miss_rate            (30.00%)
           641,769      L1-icache-load-misses:u          #      0.1 %  l1i_miss_rate            (30.00%)
        79,315,319      branch-misses:u                  #      0.1 %  branch_miss_rate         (30.00%)
    65,176,132,378      branches:u                       #   1158.2 M/sec  branch_frequency     (30.00%)
   229,604,451,070      cpu-cycles:u                     #      4.1 GHz  cycles_frequency       (40.00%)
   617,819,893,396      instructions:u                   #      2.7 instructions  insn_per_cycle  (40.00%)
     1,023,007,001      stalled-cycles-frontend:u        #     0.00 frontend_cycles_idle        (40.00%)
         4,179,549      dTLB-loads:u                     #      5.4 %  dtlb_miss_rate           (30.00%)
            20,082      iTLB-loads:u                     #     31.1 %  itlb_miss_rate           (30.00%)

      56.281917514 seconds time elapsed

      55.656189000 seconds user
       0.007932000 seconds sys
```

My implementation with SIMD:

```
 Performance counter stats for './main':

                 0      context-switches:u               #      0.0 cs/sec  cs_per_second     
                 0      cpu-migrations:u                 #      0.0 migrations/sec  migrations_per_second
               144      page-faults:u                    #     14.2 faults/sec  page_faults_per_second
         10,167.09 msec task-clock:u                     #      1.0 CPUs  CPUs_utilized       
        12,071,654      L1-dcache-load-misses:u          #      0.1 %  l1d_miss_rate            (30.00%)
           170,700      L1-icache-load-misses:u          #      0.1 %  l1i_miss_rate            (29.99%)
        26,141,333      branch-misses:u                  #      0.7 %  branch_miss_rate         (30.00%)
     3,513,154,788      branches:u                       #    345.5 M/sec  branch_frequency     (30.00%)
    41,552,877,194      cpu-cycles:u                     #      4.1 GHz  cycles_frequency       (40.01%)
    76,014,004,223      instructions:u                   #      1.8 instructions  insn_per_cycle  (40.00%)
       330,989,577      stalled-cycles-frontend:u        #     0.01 frontend_cycles_idle        (40.00%)
           331,852      dTLB-loads:u                     #     25.0 %  dtlb_miss_rate           (30.01%)
               563      iTLB-loads:u                     #    258.1 %  itlb_miss_rate           (30.00%)

      10.169321662 seconds time elapsed

      10.054647000 seconds user
       0.005937000 seconds sys
```

My implementation with SIMD & Multithreading:

```
 Performance counter stats for './main':

                 0      context-switches:u               #      0.0 cs/sec  cs_per_second     
                 0      cpu-migrations:u                 #      0.0 migrations/sec  migrations_per_second
               526      page-faults:u                    #     32.7 faults/sec  page_faults_per_second
         16,083.53 msec task-clock:u                     #     14.1 CPUs  CPUs_utilized       
        10,366,350      L1-dcache-load-misses:u          #      0.1 %  l1d_miss_rate            (29.97%)
            48,326      L1-icache-load-misses:u          #      0.1 %  l1i_miss_rate            (30.01%)
        21,927,974      branch-misses:u                  #      1.2 %  branch_miss_rate         (30.04%)
     1,892,805,254      branches:u                       #    117.7 M/sec  branch_frequency     (30.11%)
    62,920,968,445      cpu-cycles:u                     #      3.9 GHz  cycles_frequency       (40.13%)
    71,444,967,452      instructions:u                   #      1.1 instructions  insn_per_cycle  (40.14%)
       249,341,927      stalled-cycles-frontend:u        #     0.00 frontend_cycles_idle        (40.14%)
           565,409      dTLB-loads:u                     #     35.9 %  dtlb_miss_rate           (29.98%)
               663      iTLB-loads:u                     #    397.1 %  itlb_miss_rate           (29.99%)

       1.129465715 seconds time elapsed

      15.808297000 seconds user
       0.050548000 seconds sys
```
