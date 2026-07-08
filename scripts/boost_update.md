# 1. Find what's needed (run from repo root)
```
python3 scripts/scan_boost_deps.py \
    --cgal  third-party/cgal \
    --boost third-party/boost-1.88.0/include \
    --src   libsimulator libcommon
```
# 2. Download new tarball
```
curl -L https://archives.boost.io/release/X.Y.Z/source/boost_X_Y_Z.tar.gz -o /tmp/boost_X_Y_Z.tar.gz
```

# 3. Extract minimal set + generate cmake files
`scripts/extract_boost.sh /tmp/boost_X_Y_Z.tar.gz third-party/boost-X.Y.Z`