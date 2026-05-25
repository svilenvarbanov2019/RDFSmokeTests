
#include "pch.h"

extern void EngineTests();
extern void EarlyBound_IFC4_test();
extern void EarlyBound_IFC4x4_test();
extern void EarlyBound_AP242_test();
extern void EarlyBound_GuideExamples();
extern void ADBtests();
extern void Encodings();
extern void AggregationTests();
extern void Screening(void);
extern void ComplexInstance();
extern void DeriveAttrTests();
extern void SchemaReadingTests();
extern void BooleanTest();


static long HeapUsed()
{
    _HEAPINFO info = { 0, 0, 0 };
    long used = 0;
    int rc;

    while ((rc = _heapwalk(&info)) == _HEAPOK) {
        if (info._useflag == _USEDENTRY)
            used += (long) info._size;
    }
    if (rc != _HEAPEND && rc != _HEAPEMPTY)
        used = (used ? -used : -1);

    return used;
}

extern int main()
{
    try {
        printf("--------- Starting IFC engine C++ tests\n");

        //Screening();
        //return 0;

#ifdef VLD_ON
        VLDEnable();
#endif
        auto startingHeap = HeapUsed();
        
        bool stop = true;
        int i = 0;
        do
            {
            BooleanTest();
            EngineTests();
            //ComplexInstance(); #todo
            AggregationTests();
            SchemaReadingTests();
            DeriveAttrTests();
            //Encodings(); #todo
            ADBtests();
            EarlyBound_IFC4_test();
            EarlyBound_IFC4x4_test();
            EarlyBound_AP242_test();
            EarlyBound_GuideExamples();

            i++;
            if (i % 100 == 0)
                {
                auto heapSize = HeapUsed();
                printf("Cycle %d, lost memory %d\n", i, heapSize-startingHeap);
                }
        } while (!stop);

        auto finalHeap = HeapUsed();
        
        auto lostMem = finalHeap - startingHeap;
        printf("Lost memory %d bytes.\n", lostMem);
        ASSERT(lostMem <= 5000);

#ifdef VLD_ON
        VLDReportLeaks();
#endif

        printf("---------- Finished IFC enginde C++ tests.\n");
        return 0;
    }
    catch (int& code) {
        printf("!!!! IFC engine C++ tests are failed\n");
        return code;
    }
}

