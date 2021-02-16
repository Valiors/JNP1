#ifndef JNP1_ENCSTRSET_H
#define JNP1_ENCSTRSET_H

#ifdef __cplusplus

#ifndef NDEBUG
#include <iostream>
#endif // NDEBUG

#include <cstddef>

extern "C" {
    namespace jnp1 {

#else
#include <stdbool.h>
#include <stddef.h>
#endif // __cplusplus

        /* Tworzy nowy zbiór i zwraca jego identyfikator. */
        unsigned long encstrset_new();

        /* Jeżeli istnieje zbiór o identyfikatorze id, usuwa go, a w przeciwnym
        * przypadku nie robi nic. */
        void encstrset_delete(unsigned long id);

        /* Jeżeli istnieje zbiór o identyfikatorze id, zwraca liczbę jego elementów,
        * a w przeciwnym przypadku zwraca 0. */
        size_t encstrset_size(unsigned long id);

        /* Jeżeli istnieje zbiór o identyfikatorze id i element value po
        * zaszyfrowaniu kluczem key nie należy do tego zbioru, to dodaje ten
        * zaszyfrowany element do zbioru, a w przeciwnym przypadku nie robi nic.
        * Szyfrowanie jest symetryczne, za pomocą operacji bitowej XOR. Gdy klucz
        * key jest krótszy od value, to należy go cyklicznie powtórzyć. Wynikiem
        * jest true, gdy element został dodany, a false w przeciwnym przypadku. */
        bool encstrset_insert(unsigned long id, const char *value, const char *key);

        /* Jeżeli istnieje zbiór o identyfikatorze id i element value zaszyfrowany
        * kluczem key należy do tego zbioru, to usuwa element ze zbioru, a w
        * przeciwnym przypadku nie robi nic. Wynikiem jest true, gdy element został
        * usunięty, a false w przeciwnym przypadku. */
        bool encstrset_remove(unsigned long id, const char *value, const char *key);


        /* Jeżeli istnieje zbiór o identyfikatorze id i element value zaszyfrowany
        * kluczem key należy do tego zbioru, to zwraca true, a w przeciwnym
        * przypadku zwraca false. */
        bool encstrset_test(unsigned long id, const char *value, const char *key);


        /* Jeżeli istnieje zbiór o identyfikatorze id, usuwa wszystkie jego elementy,
        * a w przeciwnym przypadku nie robi nic. */
        void encstrset_clear(unsigned long id);


        /* Jeżeli istnieją zbiory o identyfikatorach src_id oraz dst_id, to kopiuje
        * zawartość zbioru o identyfikatorze src_id do zbioru o identyfikatorze
        * dst_id, a w przeciwnym przypadku nic nie robi. */
        void encstrset_copy(unsigned long src_id, unsigned long dst_id);

#ifdef __cplusplus
    }
}
#endif // __cplusplus

#endif // JNP1_ENCSTRSET_H