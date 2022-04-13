#!/bin/bash

if [ "$GITLAB_USER_EMAIL" == dev.ios@cleevio.com ]; then
    exit
fi

VERSIONSTR=$(<version)
IFS='.'
read -ra VERSION <<< "$VERSIONSTR"
MAJOR=${VERSION[0]}
MINOR=${VERSION[1]}
PATCH=${VERSION[2]}
IFS=' '

case $1 in
  major)
    MAJOR=$(($MAJOR+1))
    MINOR=0
    PATCH=0
    ;;

  minor)
    MINOR=$((MINOR+1))
    PATCH=0
    ;;

  patch)
    PATCH=$((PATCH+1))
    ;;
  *)
    PATCH=$((PATCH+1))
    ;;
esac

NEW_VERSION="$MAJOR.$MINOR.$PATCH"

echo "$NEW_VERSION" > "version"

export CI_PUSH_REPO=`echo $CI_REPOSITORY_URL | perl -pe 's#.*@(.+?(\:\d+)?)/#git@\1:#'`

git checkout -b ci_processing
git config --global user.email "dev.ios@cleevio.com"
git config --global user.name "Mac mini CI"
git remote set-url --push origin "${CI_PUSH_REPO}"

git add version
git commit -m "bump version to $NEW_VERSION"
git push origin ci_processing:${CI_COMMIT_REF_NAME}

git checkout main
git branch -D ci_processing

VEXL_IOS_FRAMEWORK="vexl_crypto_ios_xcframework.zip"
PACKAGE_REGISTRY_URL="$CI_API_V4_URL/projects/$CI_PROJECT_ID/packages/generic/vexl_crypto_ios/v$NEW_VERSION/"

echo "package url: $PACKAGE_REGISTRY_URL"

echo "curl --header \"JOB-TOKEN: $CI_JOB_TOKEN\" -T ./$VEXL_IOS_FRAMEWORK $PACKAGE_REGISTRY_URL"

curl --header "JOB-TOKEN: $CI_JOB_TOKEN" -T ./product/apple/$VEXL_IOS_FRAMEWORK $PACKAGE_REGISTRY_URL


release-cli create \
    --name "Vexl crypto library for iOS v$NEW_VERSION" \
    --tag-name "v$NEW_VERSION" \
    --ref "$CI_COMMIT_SHA" \
    --assets-link "{\"name\":\"$VEXL_IOS_FRAMEWORK\",\"url\":\"$PACKAGE_REGISTRY_URL$VEXL_IOS_FRAMEWORK\"}"
    
