builddir=cmake-build-debug

root_dir=$(cd `dirname $BASH_SOURCE[0]`/../ && pwd)
pushd $root_dir

set -e
rm -fr $builddir

supported_arch="x86 arm64 soc"

if [ $# -gt 2 ]; then
    echo "usage: "
    echo "x86: ./compile.sh x86"
    echo "soc: ./compile.sh soc sdk_dir"
    return 1
fi

verify=false
for ia in ${supported_arch[@]}
do
    if [ "$ia" == "$1" ];then
        verify=true
        break
    fi
done

if [ "$verify" == "false" ];then
    echo "usage: "
    echo "x86: ./compile.sh x86"
    echo "soc: ./compile.sh soc sdk_dir"
    exit
fi

#target_arch=$1
if [ ! -d $2 ]; then
    echo "$2 is not existed."
    exit
fi

if [ "$1" == "soc" ] && [ ! $2 ]; then
    echo "soc: build_all soc sdk_dir"
    exit
fi

function build_app() 
{
    local target_arch=$1
    local sdk_path=$(cd $2; pwd)
    rm -fr $builddir
    mkdir $builddir
    cd $builddir
    
    cmake_params="-DTARGET_ARCH=$target_arch -DUSE_QTGUI=OFF"
    
    if [ "$target_arch" == "arm64" -o "$target_arch" == "soc" ]; then
        cmake_params="$cmake_params -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-aarch64-linux.cmake -DSDK_PATH=${sdk_path}"
    elif [ "$target_arch" == "mips64" ];then
        cmake_params="$cmake_params -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mips64-linux.cmake"
    fi
    
    if [ "$1" == "client" ]; then
       cmake -DTARGET_ARCH=x86 -DUSE_SOPHON_FFMPEG=OFF -DUSE_SOPHON_OPENCV=OFF ..
    else
       cmake $cmake_params ..
    fi
    
    make -j4
    cd ..
    
}



function release_others() {
  local arch=$1
  local all_app_list="cvs10 cvs11"
  local all_jpg_app_list="cvs10 cvs11"
  for app in ${all_app_list[@]}
  do
     mkdir -p release/$app/$arch
     cp $builddir/bin/$app release/$app/$arch/
     cp ./configs/cameras.json release/$app/
     if [[ ! {${all_jpg_app_list}} = "${app}" ]]; then
       cp ./data/$app/face.jpeg release/$app/
     fi

  done
}


function build_all() {
    build_app $1 $2
    if [ "$?" == "1" ];then
        break
    fi
    release_others $1
}

build_all $1 $2 

rm -fr $builddir

popd
