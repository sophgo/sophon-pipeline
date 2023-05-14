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
    echo "arm64: ./compile.sh arm64"
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
    echo "arm64: ./compile.sh arm64"
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
    
    if [ "$target_arch" == "soc" ]; then
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
  all_app_list="video_stitch_demo yolov5s_demo retinaface_demo multi_demo facedetect_demo face_recognition_demo openpose_demo"
  all_app_list="${all_app_list} ppyoloe_demo yolact_demo"
  for app in ${all_app_list[@]}
  do
     mkdir -p release/$app/$arch
     cp $builddir/bin/$app release/$app/$arch/
     if [[ ${app} = "yolov5s_demo" ]]; then
        cp ./configs/cameras_yolov5.json release/$app/
     elif [[ ${app} = "video_stitch_demo" ]]; then
	cp ./configs/cameras_video_stitch.json release/$app/
     elif [[ ${app} = "retinaface_demo" ]]; then
	cp ./configs/cameras_retinaface.json release/$app/
     elif [[ ${app} = "multi_demo" ]]; then
        cp ./configs/cameras_multi.json release/$app/
     elif [[ ${app} = "facedetect_demo" ]]; then
	cp ./configs/cameras_face_detect.json release/$app/
     elif [[ ${app} = "face_recognition_demo" ]]; then
	cp ./configs/cameras_face_recognition.json release/$app/
     elif [[ ${app} = "openpose_demo" ]]; then
        cp ./configs/cameras_openpose.json release/$app/	
     elif [[ ${app} = "ppyoloe_demo" ]]; then
        cp ./configs/cameras_ppyoloe.json release/$app/
     elif [[ ${app} = "yolact_demo" ]]; then
	cp ./configs/cameras_yolact.json release/$app/
     else
	echo "${app} is not supported yet."
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


popd
