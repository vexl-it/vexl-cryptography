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

git fetch
git checkout main
git branch -D ci_processing

VEXL_IOS_FRAMEWORK="vexl_crypto_ios_xcframework.zip"
VEXL_ANDROID_FRAMEWORK="vexl_crypto_android_frameworks.zip"
VEXL_LINUX_FRAMEWORK="vexl_crypto_linux_frameworks.zip"
VEXL_WINDOWS_FRAMEWORK="vexl_crypto_windows_frameworks.zip"
PACKAGE_IOS_REGISTRY_URL="$CI_API_V4_URL/projects/$CI_PROJECT_ID/packages/generic/vexl_crypto_ios/v$NEW_VERSION/"
PACKAGE_ANDROID_REGISTRY_URL="$CI_API_V4_URL/projects/$CI_PROJECT_ID/packages/generic/vexl_crypto_android/v$NEW_VERSION/"
PACKAGE_LINUX_REGISTRY_URL="$CI_API_V4_URL/projects/$CI_PROJECT_ID/packages/generic/vexl_crypto_linux/v$NEW_VERSION/"
PACKAGE_WINDOWS_REGISTRY_URL="$CI_API_V4_URL/projects/$CI_PROJECT_ID/packages/generic/vexl_crypto_linux/v$NEW_VERSION/"

curl --header "JOB-TOKEN: $CI_JOB_TOKEN" -T ./product/apple/$VEXL_IOS_FRAMEWORK $PACKAGE_IOS_REGISTRY_URL
curl --header "JOB-TOKEN: $CI_JOB_TOKEN" -T ./product/android/$VEXL_ANDROID_FRAMEWORK $PACKAGE_ANDROID_REGISTRY_URL
curl --header "JOB-TOKEN: $CI_JOB_TOKEN" -T ./product/linux/$VEXL_LINUX_FRAMEWORK $PACKAGE_LINUX_REGISTRY_URL
curl --header "JOB-TOKEN: $CI_JOB_TOKEN" -T ./product/linux/$VEXL_WINDOWS_FRAMEWORK $PACKAGE_WINDOWS_REGISTRY_URL

release-cli create \
    --name "Vexl crypto library v$NEW_VERSION" \
    --tag-name "v$NEW_VERSION" \
    --ref "$CI_COMMIT_SHA" \
    --assets-link "{\"name\":\"$VEXL_IOS_FRAMEWORK\",\"url\":\"$PACKAGE_IOS_REGISTRY_URL$VEXL_IOS_FRAMEWORK\"}" \
    --assets-link "{\"name\":\"$VEXL_ANDROID_FRAMEWORK\",\"url\":\"$PACKAGE_ANDROID_REGISTRY_URL$VEXL_ANDROID_FRAMEWORK\"}" \
    --assets-link "{\"name\":\"$VEXL_LINUX_FRAMEWORK\",\"url\":\"$PACKAGE_LINUX_REGISTRY_URL$VEXL_LINUX_FRAMEWORK\"}" \
    --assets-link "{\"name\":\"$VEXL_WINDOWS_FRAMEWORK\",\"url\":\"$PACKAGE_WINDOWS_REGISTRY_URL$VEXL_WINDOWS_FRAMEWORK\"}"

curl -X POST -H 'Content-type: application/json' --data "{\"text\":\"New Release v$NEW_VERSION :tada:\n\nNew version of Vexl crypto library has been released for :apple: iOS, :android: Android, :tux: Linux and :windows: Windows \n\nCheck check out the releases in the gitlab release page.\n\nhttps://gitlab.cleevio.cz/clients/vexl-cryptography/-/releases\"}" https://hooks.slack.com/services/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
