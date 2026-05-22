/// <summary>
/// This example defines new subclass if GeometricItem, 
/// which is 3 orthogonal isosceles right triangle from coordinate origin, controlled by length property.
/// Main function is Example_RDFGeomDefine
/// </summary>

#include "pch.h"

#include "rdfgeom.h"

#include "Example_RDFGeomGet.h"

#define CLS_NAME        "Isosceles Right Triangle"
#define CLS_GEOMITEM    "GeometricItem"
#define PROP_LENGTH     "length"

static double GetLength(OwlInstance inst)
{
    OwlModel model = GetModel(inst);
    RdfProperty prop = GetPropertyByName(model, PROP_LENGTH);
    ASSERT(prop);

    double* rval = NULL;
    int64_t card = -1;
    if (0==GetDatatypeProperty(inst, prop, (const void**)&rval, &card) && rval && card > 0) {
        return rval[0];
    }

    return 0;
}

static void	MatrixIdentity(MATRIX* pOut)
{
    memset(pOut, 0, sizeof(MATRIX));
    pOut->_11 = pOut->_22 = pOut->_33 = 1.;
}

static void ExpandRange(VECTOR3* startVector, VECTOR3* endVector, const VECTOR3& pt)
{
    startVector->x = std::min(startVector->x, (double)pt.x);
    startVector->y = std::min(startVector->y, (double)pt.y);
    startVector->z = std::min(startVector->z, (double)pt.z);
    endVector->x = std::max(endVector->x, (double)pt.x);
    endVector->y = std::max(endVector->y, (double)pt.y);
    endVector->z = std::max(endVector->z, (double)pt.z);
}

/// <summary>
/// Callback gets bounding box of the triangle - this is square with 'length' size
/// </summary>
static bool CB_GetBoundingBox(OwlInstance inst, void*)
{
    VECTOR3 startVector;
    VECTOR3 endVector;
    MATRIX  transform;

    MatrixIdentity(&transform);

    startVector.x = startVector.y = startVector.z = DBL_MAX;
    endVector.x = endVector.y = endVector.z = -DBL_MAX;

    if (SHELL* shell = rdfgeom_GetBRep(inst)) {
        
        //even it may have no sense for this example - when shell is ready we can use it to calculate bounding box
        
        int_t Npoints = rdfgeom_GetNumOfPoints(shell);
        VECTOR3* rpt = rdfgeom_GetPoints(shell);

        for (int_t i = 0; i < Npoints; i++) {
            ExpandRange(&startVector, &endVector, rpt[i]);
        }
    }
    else {

        //if shell is not created yet we can base on property
        
        double length = GetLength(inst);

        startVector.x = startVector.y = startVector.z = 0;
        endVector.x = endVector.y = endVector.z = length;
    }

    if (endVector.x >= startVector.x) {
        rdfgeom_SetBoundingBox(inst, &startVector, &endVector, &transform);
        return true;
    }
    else {
        rdfgeom_SetBoundingBox(inst, 0,0,0);
        return false;
    }
}

/// <summary>
/// This callback creates the shell - 3 conceptual faces with 1 triangular face each
/// </summary>
static void CB_InitShell(OwlInstance inst, void*)
{
    if (double len = GetLength(inst)) {
        if (SHELL* shell = rdfgeom_GetBRep(inst)) {

            //we need 4 points 
            // 0,   0,   0
            // len, 0,   0
            // 0,   len, 0
            // 0,   0,   len

            rdfgeom_AllocatePoints(inst, shell, 4, false, false);

            VECTOR3* points = rdfgeom_GetPoints(shell);
             
            for (int i = 0; i < 4; i++) {
                VECTOR3& pt = points[i];
               
                pt.x = pt.y = pt.z = 0; //set all to 0
                if (i > 0) { //and than set i-th coordinate to len
                    double* xyz = &pt.x;
                    xyz[i - 1] = len;
                }
            }

            //now will create 3 faces
            //   0-1-2
            //   0-1-3 
            //   0-2-3
            CONCEPTUAL_FACE** cfacePP = rdfgeom_GetConceptualFaces(shell);
            for (int_t i = 0; i < 3; i++) {
                rdfgeom_cface_Create(inst, cfacePP);

                STRUCT_FACE** facePP = rdfgeom_cface_GetFaces(*cfacePP);
                rdfgeom_face_Create(inst, facePP);

                STRUCT_VERTEX** vertexPP = rdfgeom_face_GetBoundary(*facePP);
                for (int ind = 0; ind < 4; ind++) {

                    rdfgeom_vertex_Create(inst, vertexPP, ind, false);
                    vertexPP = rdfgeom_vertex_GetNext(*vertexPP);

                    if (ind == 2 - i)
                        ind++;
                }

                //add last closing point to boundary
                rdfgeom_vertex_Create(inst, vertexPP, 0, true);
            }
        }
    }
}

/// <summary>
/// 
/// </summary>
static OwlClass Example_RDFGeomDefine(OwlModel model)
{
    OwlClass cls = CreateClass(model, CLS_NAME);

    OwlClass geomItem = GetClassByName(model, CLS_GEOMITEM);
    ASSERT(geomItem);
    SetClassParent(cls, geomItem);

    RdfProperty prop = GetPropertyByName(model, PROP_LENGTH);
    ASSERT(prop);
    SetClassPropertyCardinalityRestriction(cls, prop, 1, 1);

    //
    // Set callbacks to define class geometry
    //
    rdfgeom_SetClassGeometry(cls, CB_InitShell, CB_GetBoundingBox, NULL);

    return cls;
}

/// <summary>
/// run the test
/// </summary>
extern void Test_Example_RDFGeomDefine()
{
    ENTER_TEST;

    auto model = CreateModel();

    auto cls = Example_RDFGeomDefine(model);
    ASSERT(cls);

    auto inst = CreateInstance(cls);
    ASSERT(inst);

    RdfProperty prop = GetPropertyByName(model, PROP_LENGTH);
    ASSERT(prop);

    SetDatatypeProperty(inst, prop, 1.8);

    SaveModel(model, "Triangle.bin");

    //
    double expected[6] = { 0,0,0,1.8,1.8,1.8 };
    double box[6];

    auto ok = GetBoundingBox(inst, box, box + 3);
    ASSERT(ok);
    ASSERT_ARR_EQ(box, expected, 6);

    //
    CalculateInstance(inst);

    //
    SomeStatistic stat;
    memset(&stat, 0, sizeof(SomeStatistic));
    Example_RDFGeomGet(inst, stat);
    ASSERT(stat.numOfPoints == 4 && stat.numOfConceptualFaces == 3 && stat.numOfFaces == 3 && stat.numOfVertices == 12);
    ASSERT(stat.sumOfCoordinates.x == 1.8 && stat.sumOfCoordinates.y == 1.8 && stat.sumOfCoordinates.z == 1.8);

    ok = GetBoundingBox(inst, box, box + 3);
    ASSERT(ok);
    ASSERT_ARR_EQ(box, expected, 6);

    CloseModel(model);
}