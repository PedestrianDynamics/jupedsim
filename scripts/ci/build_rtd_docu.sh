#!/bin/bash
set -ex

rtd_version=${branch}

# Check if current commit is tagged
if git describe --exact-match; then
    rtd_version=`git describe --exact-match`
else
    # if branch starts with pull it's a pull request, and
    [[ ${branch} = pull/* ]] && rtd_version=${branch#*/}
fi

export GIT_SHA=`git rev-parse HEAD`

pip install . --break-system-packages
sphinx-build readthedocs/source readthedocs/build -j$(nproc)
tar -cvf readthedocs/${GIT_SHA}.tar readthedocs/build
curl -k -T readthedocs/${GIT_SHA}.tar -u ${sciebo_token}: https://fz-juelich.sciebo.de/public.php/webdav/${GIT_SHA}.tar --fail-with-body
curl \
    -X POST \
    -H "Authorization: token ${rtd_token}" https://readthedocs.org/api/v3/projects/jupedsim/versions/${rtd_version}/builds/ \
    --fail-with-body
