DIR=parallel_builder/


build: FORCE
	bash -c "ml purge && ml intel-compilers/2024.2.0 CMake/3.23.1-GCCcore-11.3.0 && cd build && make -j"
	
vtune: FORCE
	bash -c "sbatch vtune.sl"
	mv slurm-*.out sbatch_out/

runtree: build
	bash -c "ml purge && ml intel-compilers/2024.2.0 CMake/3.23.1-GCCcore-11.3.0 && cd build && ./PMC -b tree ../data/bun_zipper_res1.pts bun_zipper_res4.obj"

runtree16: build
	bash -c "ml purge && ml intel-compilers/2024.2.0 CMake/3.23.1-GCCcore-11.3.0 && cd build && ./PMC -b tree -t 16 ../data/bun_zipper_res1.pts bun_zipper_res4.obj"


runloop: build
	bash -c "ml purge && ml intel-compilers/2024.2.0 CMake/3.23.1-GCCcore-11.3.0 && cd build && ./PMC -b loop ../data/bun_zipper_res1.pts bun_zipper_res1.obj"

runloop16: build
	bash -c "ml purge && ml intel-compilers/2024.2.0 CMake/3.23.1-GCCcore-11.3.0 && cd build && ./PMC -b loop -t 16 ../data/bun_zipper_res1.pts bun_zipper_res1.obj"


FORCE: ;




pack: $(DIR)loop_mesh_builder.h $(DIR)loop_mesh_builder.cpp $(DIR)tree_mesh_builder.h $(DIR)tree_mesh_builder.cpp PMC-xnovak3g.txt 3_4.txt 4_1.txt 4_2.txt 4_3.txt input_scaling_strong.png input_scaling_weak.png grid_scaling.png
	rm -f xnovak3g.zip
	zip -j xnovak3g.zip $^