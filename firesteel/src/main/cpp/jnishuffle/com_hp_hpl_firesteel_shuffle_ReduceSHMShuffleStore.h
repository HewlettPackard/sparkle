/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore */

#ifndef _Included_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
#define _Included_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    ninitialize
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_ninitialize
  (JNIEnv *, jobject, jlong, jint, jint, jint);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nstop
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nstop
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nshutdown
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nshutdown
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nmergeSort
 * Signature: (JIILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/ReduceStatus;IILjava/nio/ByteBuffer;IZZ)J
 */
JNIEXPORT jlong JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nmergeSort
  (JNIEnv *, jobject, jlong, jint, jint, jobject, jint, jint, jobject, jint, jboolean, jboolean);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    ncreateShuffleStore
 * Signature: (JIILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/ReduceStatus;ILjava/nio/ByteBuffer;IZZ)J
 */
JNIEXPORT jlong JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_ncreateShuffleStore
  (JNIEnv *, jobject, jlong, jint, jint, jobject, jint, jobject, jint, jboolean, jboolean);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    ngetKValueTypeId
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_ngetKValueTypeId
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    ngetKValueType
 * Signature: (J)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_ngetKValueType
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    ngetVValueType
 * Signature: (J)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_ngetVValueType
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetKVPairs
 * Signature: (Ljava/nio/ByteBuffer;I[II)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetKVPairs
  (JNIEnv *, jobject, jobject, jint, jintArray, jint);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetSimpleKVPairs
 * Signature: (J[Ljava/lang/Object;Ljava/nio/ByteBuffer;I[II)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetSimpleKVPairs
  (JNIEnv *, jobject, jlong, jobjectArray, jobject, jint, jintArray, jint);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetKVPairsWithIntKeys
 * Signature: (JLjava/nio/ByteBuffer;IILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/MergeSortedResult;)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetKVPairsWithIntKeys
  (JNIEnv *, jobject, jlong, jobject, jint, jint, jobject);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetSimpleKVPairsWithIntKeys
 * Signature: (JLjava/nio/ByteBuffer;IILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/MergeSortedResult;)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetSimpleKVPairsWithIntKeys
  (JNIEnv *, jobject, jlong, jobject, jint, jint, jobject);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetKVPairsWithFloatKeys
 * Signature: (JLjava/nio/ByteBuffer;IILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/MergeSortedResult;)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetKVPairsWithFloatKeys
  (JNIEnv *, jobject, jlong, jobject, jint, jint, jobject);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetSimpleKVPairsWithFloatKeys
 * Signature: (JLjava/nio/ByteBuffer;IILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/MergeSortedResult;)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetSimpleKVPairsWithFloatKeys
  (JNIEnv *, jobject, jlong, jobject, jint, jint, jobject);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetKVPairsWithLongKeys
 * Signature: (JLjava/nio/ByteBuffer;IILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/MergeSortedResult;)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetKVPairsWithLongKeys
  (JNIEnv *, jobject, jlong, jobject, jint, jint, jobject);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetSimpleKVPairsWithLongKeys
 * Signature: (JLjava/nio/ByteBuffer;IILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/MergeSortedResult;)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetSimpleKVPairsWithLongKeys
  (JNIEnv *, jobject, jlong, jobject, jint, jint, jobject);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetKVPairsWithByteArrayKeys
 * Signature: (JLjava/nio/ByteBuffer;IILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/MergeSortedResult;)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetKVPairsWithByteArrayKeys
  (JNIEnv *, jobject, jlong, jobject, jint, jint, jobject);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetSimpleKVPairsWithByteArrayKeys
 * Signature: (JLjava/nio/ByteBuffer;IILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/MergeSortedResult;)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetSimpleKVPairsWithByteArrayKeys
  (JNIEnv *, jobject, jlong, jobject, jint, jint, jobject);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetKVPairsWithStringKeys
 * Signature: (JLjava/nio/ByteBuffer;IILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/MergeSortedResult;)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetKVPairsWithStringKeys
  (JNIEnv *, jobject, jlong, jobject, jint, jint, jobject);

/*
 * Class:     com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore
 * Method:    nGetSimpleKVPairsWithStringKeys
 * Signature: (JLjava/nio/ByteBuffer;IILcom/hp/hpl/firesteel/shuffle/ShuffleDataModel/MergeSortedResult;)I
 */
JNIEXPORT jint JNICALL Java_com_hp_hpl_firesteel_shuffle_ReduceSHMShuffleStore_nGetSimpleKVPairsWithStringKeys
  (JNIEnv *, jobject, jlong, jobject, jint, jint, jobject);

#ifdef __cplusplus
}
#endif
#endif
