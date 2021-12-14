# delete build folder
rm -rf build/

#creat build folder
mkdir -p ./build/

# makefile build
cd build
cmake ..
cmake --build ./
make
cd -

if [ -n "$2" ]; then
    output_dir=$2
else
    output_dir=../h_files
fi
mkdir -p $output_dir

# h_file=$output_dir/$(basename $1 .csv)
h_file=$(basename $1 .csv)
echo $h_file

# run application Linux
./build/csv2bf $1 $h_file

# run astyle format code
astyle $h_file.h
# clang-format -style=File -i ./$h_file.h
