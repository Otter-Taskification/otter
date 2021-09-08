!credit: https://github.com/LonelyCat124
PROGRAM omp_parallel_for
USE OMP_LIB

        integer, dimension(20) :: num
        integer :: k
        num(:) = 0

        !$OMP PARALLEL
        !$OMP SINGLE
        do k=1, 20
                num(k) = omp_get_thread_num()
        end do
        !$OMP END SINGLE nowait
        !$OMP END PARALLEL

END PROGRAM
