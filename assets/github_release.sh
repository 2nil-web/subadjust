#!/bin/bash

## To create a new delivery
# Check remote tags   :
#    git ls-remote --tags origin # default is for github
#    git ls-remote --tags gitlab # non default is for gitlab
# Check local tags    :
#    git describe --abbrev=0 --tags
# New version tag     : git tag -a X.Y.Z-nom_de_la_prerelease -m 'commentaire' # De préférence un tag annoté (-a).
# Push a tag          : git push --tags
# Delivery            : make deliv

# Get current branch  : git rev-parse --abbrev-ref HEAD
# Get current tag     : git describe --abbrev=0 --tags

# See managing releases: https://docs.github.com/en/rest/releases/releases
# See managing assets: https://docs.github.com/fr/rest/releases/assets

SCRIPT=$(realpath "$0")
# <=> SCRIPT="$(readlink --canonicalize-existing "$0")"
SCRIPTPATH=$(dirname "$SCRIPT")
. ${SCRIPTPATH}/github_access.sh

function gh_curr_rel () {
  asset_path=$1
  asset_name=$(basename $1)
  repo_git=$(git config --get remote.origin.url)
  repo_name=$(basename -s .git ${repo_git})

  if [ "${OS}" = "Windows_NT" ]
  then
    SYS_VER=${OS}_$(powershell -Command '(Get-WmiObject -class Win32_OperatingSystem).Version')
    #SYS_VER=$(systeminfo | grep -i version | grep -i syst | sed 's/.*: *\(.*\) *.*/\1/')
  else
    OS_ID=$(sed -n 's/^ID=//p' /etc/os-release)
    if [ ${OS_ID} = arch ]
    then
      SYS_VER=Arch_Linux_$(sed -n 's/^VERSION_ID=//p' /etc/os-release)
    else
      SYS_VER=$(lsb_release -irs | sed 'N;s/\n/_/' | sed 's/ /_/g')
    fi
  fi

  branch_name=$(git rev-parse --abbrev-ref HEAD)
  tag_name=$(git describe --abbrev=0 --tags)
  rel_name=${repo_name}-${tag_name}

  # Si on n'est pas sur la branche master (gitlab pre 2021-06-22) ou main (gitlab post 2021-06-22 et github) alors on se considére en pre-release et on ajoute le nom de la branche pour le dénoter
  if [ ${branch_name} != master ] && [ ${branch_name} != main ]
  then
    rel_name+=-${branch_name}
  fi

  if true; then
  echo "System         : ${SYS_VER}"
  echo "Current branch : ${branch_name}"
  echo "Current tag    : ${tag_name}"
  echo "Release name   : ${rel_name}"
  echo "Asset path     : $asset_path"
  echo "Asset name     : $asset_name"
  echo
  fi

  gh_rel_endp="${gh_api_endp}/repos/${gh_user}/${repo_name}/releases"

  # is there already a github release for the current tag ?
  exists_rel=$(curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp} | jq -r '.[]|.name,.id' | sed 'N;s/\n/;/' | grep ${rel_name})

  #echo "exists_rel: $exists_rel"

  # If no then create it
  if [ -z "$exists_rel" ]
  then
    #echo "The release ${rel_name} does not exist yet, creating it."
    rel_post="{\"tag_name\":\"${tag_name}\",\"target_commitish\":\"master\",\"name\":\"${rel_name}\",\"body\":\"Description of the release\",\"draft\":false,\"prerelease\":false,\"generate_release_notes\":false}"
    curl -s -X POST -H "Authorization: Bearer ${gh_tok}" ${gh_rel_endp} -d "${rel_post}" >/dev/null
    sleep 2
  fi

  RELEASE_ID=$(curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp} | jq -r '.[]|.name,.id' | sed 'N;s/\n/;/' | sed -n "s/${rel_name};//p") >/dev/null

  # Get a specific release upload_url
  gh_upl_url=$(curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp} | jq -r '.[]|.name,.upload_url' | sed 'N;s/\n/;/;s/{?name,label}//' | sed -n "s/${rel_name};//p")

  #echo "RELEASE_ID: $RELEASE_ID"
  #echo "Upload url: $gh_upl_url"

  post_res=$(curl -s -X POST -H "Authorization: Bearer ${gh_tok}" -H "Content-Type: application/octet-stream" ${gh_upl_url}?name=${asset_name} --data-binary "@${asset_path}")

  if grep 'errors.*code' <<<${post_res} >/dev/null
  then
    upl_res=$(echo ${post_res} | jq -r 'select(.errors[].code != null)|.errors[].code')
  fi

  # If post return any error code "already_exists" then there is a need to update an already existing release
  if [ "$upl_res" = "already_exists" ]
  then
    echo "Updating an already existing github release asset."
    # Essayer de cacher le message "error (at <stdin>:0): Cannot iterate over null (null)"
    ASSET_ID=$(curl -sH "Authorization: Bearer ${gh_tok}" ${gh_rel_endp}/${RELEASE_ID}/assets | jq -r '.[]|.name,.id' | sed 'N;s/\n/;/' | sed -n "s/${asset_name};//p") >/dev/null
    asset_post="{\"name\":\"$asset_name\",\"label\":\"$asset_name\"}"

    #echo "asset_post: $asset_post"
    #echo "ASSET_ID  : $ASSET_ID"
    curl -sX PATCH -H "Authorization: Bearer ${gh_tok}" ${gh_rel_endp}/assets/${ASSET_ID} -d "${asset_post}" >/dev/null
  else
    echo "Uploading a new github release asset."
  fi

	echo "Done, to see it go there https://github.com/${gh_user}/subadjust/releases"
}

gh_curr_rel $1

