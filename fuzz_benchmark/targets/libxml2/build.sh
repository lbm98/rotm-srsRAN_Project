export V=1


git clone --branch v2.12.0 --depth 1https://gitlab.gnome.org/GNOME/libxml2.git

./autogen.sh \
    --disable-shared \
    --without-debug \
    --without-ftp \
    --without-http \
    --without-legacy \
    --without-python

make -j$(nproc)

cd fuzz
make clean-corpus
make fuzz.o

make xml.o
# Link with $CXX
$CXX $CXXFLAGS \
    xml.o fuzz.o \
    -o $OUT/xml \
    $LIB_FUZZING_ENGINE \
    ../.libs/libxml2.a -Wl,-Bstatic -lz -llzma -Wl,-Bdynamic

[ -e seed/xml ] || make seed/xml.stamp
zip -j $OUT/xml_seed_corpus.zip seed/xml/*
