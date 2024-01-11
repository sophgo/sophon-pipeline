builddir=cmake-build-debug
root_dir=$(cd `dirname $BASH_SOURCE[0]`/../ && pwd)
pushd $root_dir

set -e
rm -fr $builddir

supported_arch="x86 arm64 soc"

if [ $# -gt 3 ]; then
    echo "usage: "
    echo "x86: ./compile.sh x86"
    echo "soc: ./compile.sh soc sdk_dir qt_dir"
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
    cmake_params="-DTARGET_ARCH=$target_arch"
    if [ -n "$5" ]; then
        pwd
        local qt_path=$(cd $5; pwd)
        cmake_params="$cmake_params -DQT_PATH=$qt_path"
    fi
    rm -fr $builddir
    mkdir $builddir
    cd $builddir

    if [ "$target_arch" == "arm64" -o "$target_arch" == "soc" ]; then
        cmake_params="$cmake_params -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-aarch64-linux.cmake -DSDK_PATH=${sdk_path}"
    elif [ "$target_arch" == "mips64" ];then
        cmake_params="$cmake_params -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mips64-linux.cmake"
    else 
        cmake_params="$cmake_params -DA2_SDK=off" 
    fi
    
    if [ "$1" == "client" ]; then
       cmake -DTARGET_ARCH=x86 -DUSE_SOPHON_FFMPEG=OFF -DUSE_SOPHON_OPENCV=OFF ..
    else
       cmake $cmake_params $3 ..
    fi
    
    make -j4
    if [ ! -d ../test_execs ]; then
        mkdir ../test_execs
    fi
    cp bin/cvs20 ../test_execs/$4
    cd ..
}



function release_others() {
  local arch=$1
  all_app_list="cvs20"
  local all_jpg_app_list="cvs20"
  for app in ${all_app_list[@]}
  do
     mkdir -p release/$app/$arch
     cp $builddir/bin/$app release/$app/$arch/
     cp ./configs/cameras_cvs.json release/$app/
     for jpg_app in ${all_jpg_app_list[@]}
     do
        if [[  "${jpg_app}" = "${app}" ]]; then
           cp ./data/$app/face.jpeg release/$app/
        fi
     done
  done
}


function build_all() {
    # build_app $1 $2 "-DUSE_QTGUI=ON -DWITH_DECODE=OFF -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF" cvs20_widget $3
    # build_app $1 $2 "-DUSE_QTGUI=OFF -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=OFF" cvs20_detector
    # build_app $1 $2 "-DUSE_QTGUI=OFF -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=ON" cvs20_extractor
    # build_app $1 $2 "-DUSE_QTGUI=OFF -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DWITH_OUTPUTER=ON" cvs20_all_client

    # build_app $1 $2 "-DUSE_QTGUI=OFF -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF -DTEST_PREPROCESS=ON" cvs20_decode_preprocess $3
    # build_app $1 $2 "-DUSE_QTGUI=ON -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF -DWITH_ENCODE=OFF -DWITH_HDMI=ON" cvs20_decode_gui $3
    
    build_app $1 $2 "-DUSE_QTGUI=ON -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF -DTEST_PREPROCESS=ON -DWITH_HDMI=ON" cvs20_decode_gui_preprocess $3
    build_app $1 $2 "-DUSE_QTGUI=OFF -DWITH_DECODE=ON -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF" cvs20_decode
    build_app $1 $2 "-DUSE_QTGUI=OFF -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_JPEG=ON" cvs20_enc_jpeg $3
    build_app $1 $2 "-DUSE_QTGUI=OFF -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_H264=ON" cvs20_enc_h264 $3
    build_app $1 $2 "-DUSE_QTGUI=OFF -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON" cvs20_all
    build_app $1 $2 "-DUSE_QTGUI=OFF -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_JPEG=ON" cvs20_all_enc
    build_app $1 $2 "-DUSE_QTGUI=ON -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON" cvs20_all_gui $3
    build_app $1 $2 "-DUSE_QTGUI=ON -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_H264=ON" cvs20_all_gui_enc_h264 $3
    build_app $1 $2 "-DUSE_QTGUI=ON -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_H264=ON -DWITH_JPEG_160FPS=ON" cvs20_all_gui_enc_h264_and_jpeg160fps $3
    build_app $1 $2 "-DUSE_QTGUI=ON -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_H264=ON -DWITH_JPEG_160FPS=ON -DUSE_2CORE=ON" cvs20_all_gui_enc_h264_and_jpeg160fps_2core $3
    # build_app $1 $2 "-DUSE_QTGUI=ON -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=OFF -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_H264=ON -DWITH_JPEG_160FPS=ON" cvs20_detector_gui_enc_h264_and_jpeg160fps $3
    # build_app $1 $2 "-DUSE_QTGUI=ON -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_JPEG=ON" cvs20_all_gui_enc_jpg $3

    # build_app $1 /home/lihengfang/work/sophon-pipeline/soc-sdk-230501 \
    # "-DA2_SDK=OFF -DUSE_SOPHON_OPENCV=ON -DUSE_QTGUI=OFF -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF -DTEST_PREPROCESS=ON" cvs20_decode_preprocess_se5
    # build_app $1 /home/lihengfang/work/sophon-pipeline/soc-sdk-230501 \
    # "-DA2_SDK=OFF -DUSE_QTGUI=OFF -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DUSE_SOPHON_OPENCV=ON" \
    # cvs20_all_se5
    # build_app $1 /home/lihengfang/work/sophon-pipeline/soc-sdk-230501 \
    # "-DA2_SDK=OFF -DUSE_QTGUI=ON -DWITH_ENCODE=ON -DWITH_HDMI=ON -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF -DUSE_SOPHON_OPENCV=ON" \
    # cvs20_gui_se5 ~/work/sophon-QT/HDMIDemo/install/
    # build_app $1 /home/lihengfang/work/sophon-pipeline/soc-sdk-230501 \
    # "-DA2_SDK=OFF -DUSE_QTGUI=OFF -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DUSE_SOPHON_OPENCV=ON -DWITH_OUTPUTER=ON" \
    # cvs20_all_client_se5
    # build_app $1 /home/lihengfang/work/sophon-pipeline/soc-sdk-230501 \
    # "-DA2_SDK=OFF -DUSE_QTGUI=ON -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DUSE_SOPHON_OPENCV=ON" \
    # cvs20_all_gui_se5 ~/work/sophon-QT/HDMIDemo/install/
    # build_app $1 /home/lihengfang/work/sophon-pipeline/soc-sdk-230501 \
    # "-DA2_SDK=OFF -DUSE_QTGUI=ON -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DUSE_SOPHON_OPENCV=ON -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_JPEG=ON" \
    # cvs20_all_gui_enc_jpg_se5 ~/work/sophon-QT/HDMIDemo/install/
    # build_app $1 /home/lihengfang/work/sophon-pipeline/soc-sdk-230501 \
    # "-DA2_SDK=OFF -DUSE_QTGUI=ON -DWITH_DETECTOR=ON -DWITH_EXTRACTOR=ON -DUSE_SOPHON_OPENCV=ON -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_H264=ON" \
    # cvs20_all_gui_enc_h264_se5 ~/work/sophon-QT/HDMIDemo/install/
    # build_app $1 /home/lihengfang/work/sophon-pipeline/soc-sdk-230501 \
    # "-DA2_SDK=OFF -DUSE_QTGUI=OFF -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF -DUSE_SOPHON_OPENCV=ON -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_JPEG=ON" \
    # cvs20_enc_jpg_se5 ~/work/sophon-QT/HDMIDemo/install/
    # build_app $1 /home/lihengfang/work/sophon-pipeline/soc-sdk-230501 \
    # "-DA2_SDK=OFF -DUSE_QTGUI=OFF -DWITH_DETECTOR=OFF -DWITH_EXTRACTOR=OFF -DUSE_SOPHON_OPENCV=ON -DWITH_ENCODE=ON -DWITH_HDMI=OFF -DENC_H264=ON" \
    # cvs20_enc_h264_se5 ~/work/sophon-QT/HDMIDemo/install/
    if [ "$?" == "1" ];then
        break
    fi
    # release_others $1
}

build_all $1 $2 $3
git log > cvs20_version.txt
echo "$2" > soc_sdk_version.txt
mv *_version.txt test_execs
tar cvf test_execs.tar test_execs
# cp -r test_execs release/cvs20/

popd
