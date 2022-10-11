#!/bin/bash

script_dir=$(dirname $(readlink -f "$0"))
project_dir=${script_dir}/..
project_name=sophon-pipeline
set -e

pushd ${project_dir}

commit_id=$(git log -1 | awk 'NR==1 {print substr($2,0,8)}')
times=`date +%Y%m%d`

SOPHON_PIPELINE_VERSION=`grep "project" CMakeLists.txt | grep "${project_name}" |awk -F '_v' '{print $2}' |awk -F ')' '{print $1}'`
dst_file_name_prefix="${project_name}_v${SOPHON_PIPELINE_VERSION}_${commit_id}_${times}"
dst_file_name="${dst_file_name_prefix}.tar.gz"

dst_dir=$project_dir/out
sophon_pipeline_dir=${dst_dir}/${dst_file_name_prefix}

rm -rf ${dst_dir}
mkdir ${dst_dir}
mkdir ${sophon_pipeline_dir}

cp -r `ls ${project_dir} -A | grep -v "out"` ${sophon_pipeline_dir}/
rm -rf ${sophon_pipeline_dir}/.git

pushd ${dst_dir}
tar -cvzf ${dst_file_name} ${dst_file_name_prefix}
rm -rf ${sophon_pipeline_dir}
popd

popd

echo "saved: ${dst_dir}/${dst_file_name}"