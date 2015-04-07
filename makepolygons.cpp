#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <stdio.h>

#include "gdal.h"
#include "ogr_api.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"

#ifdef USE_XALAN
#include "xalanc/Include/PlatformDefinitions.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xalanc/XalanTransformer/XalanTransformer.hpp"
#endif /* USE_XALAN */

#ifdef USE_LIBXSLT
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
//#include <libxml/DOCBparser.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif /* USE_LIBXSLT */


#include "util.h"
#include "primitives.h"
#include "region.h"
#include "rasterutil.h"
#include "raster.h"
#include "thresholdraster.h"
#include "regionmap.h"



using namespace std;



const string usageStr = (string)    "MakePolygons_C++.exe\n" +
                                    "    Sonoma Technology, Inc. (STI) -- DAP/JER/KDU\n" +
                                    "    Copyright 2009\n" +
                                    "\n" +
                                    "Usage:\n" +
                                    "    MakePolygons [arguments]\n" +
                                    "\n" +
                                    "Available arguments:\n" +
                                    "    -in=<raster>         Input raster data\n" +
                                    "    -band=<num>          Select raster band (for multi-band datasets)\n" +
                                    "    -cutpoints=<file>    Select polygon cutpoints (CSV file)\n" +
                                    "    -noData=<value>      Treat cells with given value as null\n" +
                                    //"    -dump=<file.tif>     Dump intermediate region map (in GeoTiff format)\n" +
                                    "    -assumeproject=<proj>   Force incoming data to be treated as given projection\n" +
                                    "    -reproject=<proj>    Reproject to given projection (e.g. EPSG:4326)\n" +
                                    //"    -resample=<N>        Resample the data to N times resolution\n" +
                                    //"    -resampleMethod=<X>  Use resample method X (default: Bilinear)\n" +
                                    //"    -interp=<method>     Sub-pixel interpolation method (default: Planar)\n" +
                                    //"    -threshold=<method>  Sub-pixel threshold method (default: Mean)\n" +
                                    "    -format=<OGR driver> Output format for polygon data (default: Shapefile)\n" +
                                    "    -kmlStyle=<file.xsl> Apply the given XSL transform (implies -format=KML)\n" +
                                    "    -out=<filename>      Output polygon file name\n" +
                                    "\n";




string* inDataFile = NULL;
string* cutpointsFile = NULL;
int bandNum = 1;
string* outFile = NULL;
double noDataValue = NULL_DOUBLE_;
OGRSpatialReference* inSpatialRef = NULL;
OGRSpatialReference* outSpatialRef = NULL;
string* ogrDriver = new string("ESRI Shapefile");
string* kmlStyleFile = NULL;
CutpointScale* cutpoints = NULL;
GDALDataset* gdalDataset = NULL;



GDALDataset* openGDALDataSet()
{
    cout << "Opening " << inDataFile->c_str() << "...\n";

    GDALDataset* ds = (GDALDataset *)GDALOpen(inDataFile->c_str(), GA_ReadOnly);
    if (ds != NULL) {

        GDALDriver* drv = ds->GetDriver();
        if (drv == NULL) {
            cout << " -- Unable to find GDAL driver for " << (char *)inDataFile->c_str() << " -- ";
            ds = NULL;
        } else {

            cout << "GDAL FILE TYPE: " << drv->GetDescription() << "\n";
            cout << "CONTAINS " << ds->GetRasterCount() << " dataset(s); size " << ds->GetRasterXSize() << "x" << ds->GetRasterYSize() << "\n";
            if (ds->GetRasterCount() == 0) {
                ds = NULL;
                cout << " -- FILE " << (char *)inDataFile->c_str() << " contains zero raster bands -- ";
            }
        }

    } else
        cout << " -- Unable to open " << (char *)inDataFile->c_str() << " -- ";

    cout << "...Done.\n";

    return ds;
}



bool initGDAL()
{
    cout << "Registering GDAL...";

    try {
        GDALAllRegister();
        OGRRegisterAll();
    } catch (exception& e) {
        cout << e.what();
        return false;
    }

    cout << "...Done\n";
    return true;
}



CutpointScale* readCutpoints()
{
    cout << "reading cutpoints...";

    vector<Category*>* cuts = new vector<Category*>();
    ifstream inFile(cutpointsFile->c_str());
    string line;
    while (getline (inFile, line))
    {
        istringstream linestream(line);

        string* name = new string();
        getline (linestream, *name, ',');
        trim(*name);

        string* threshold = new string;
        getline (linestream, *threshold, ',');
        trim(*threshold);

        if (isNumber(threshold))
            cuts->insert(cuts->end(), new Category(name, atof(threshold->c_str())));
        else
            cout << " -- ignoring \"" << line.c_str() << "\" -- ";

        delete threshold;
    }

    cout << "...Done.\n";
    if (cuts->empty())
        return NULL;
    CutpointScale* cutpointScale = new CutpointScale(cuts);
    delete cuts;
    return cutpointScale;
}



string* parseCommandLine(int argc, const char* argv[])
{
    char *saveProjection = NULL;

    for(int i = 1; i < argc; i++) {
        string item(argv[i]);

        if(item == "-h" || item == "-help" || item == "-?" || item == "/help" || item == "/h" || item == "/?")
            return new string(usageStr);

        if (item[0] == '-' && strrchr(item.c_str(), '=') != NULL) {

            int eqPos = item.find('=');
            string arg = item.substr(1, eqPos - 1);
            toLowerCase(&arg);

            string args[] = { string("in"), string("cutpoints"), string("out"), string("band"), string("format"), string("kmlstyle") };
            string* found = find(args, args + 5, arg);
            if (!(*found == arg))
                return new string(string("Unnkown argument: ") + arg + string("\n\n"));

            string value = item.substr(eqPos + 1);

            if (arg == "in"){

                inDataFile = new string(value);

            } else if (arg == "band") {

                if (isNumber(&value))
                    bandNum = atoi(value.c_str());
                else
                    return new string("can't parse band number\n");

            } else if (arg == "cutpoints") {

                cutpointsFile = new string(value);

            } else if (arg == "out") {

                outFile = new string(value);

            } else if (arg == "assumeproject") {

                inSpatialRef = new OGRSpatialReference("");
                inSpatialRef->SetFromUserInput(value.c_str());
                saveProjection = (char *)value.c_str();

            } else if (arg == "reproject") {

                if (outSpatialRef != NULL)
                    cout << "Ignoring 'reproject' argument " << value.c_str() << ", projection already set to " << saveProjection << "\n";
                else {
                    outSpatialRef = new OGRSpatialReference("");
                    outSpatialRef->SetFromUserInput(value.c_str());
                }

            } else if (arg == "format") {

                ogrDriver = new string(value);
                if (value == "KML") {
                    if (outSpatialRef != NULL)
                        cout << "Ignoring previously set projection " << saveProjection << ", setting to WGS84 for KML\n";
                    else
                        outSpatialRef = new OGRSpatialReference("");
                    outSpatialRef->SetFromUserInput("WGS84");
                    saveProjection = (char *)"WGS84";
                }

            } else if (arg == "kmlstyle") {

                ogrDriver = new string("KML");
                if (outSpatialRef != NULL)
                    cout << "Ignoring previously set projection " << saveProjection << ", setting to WGS84 for KML\n";
                else
                    outSpatialRef = new OGRSpatialReference("");
                outSpatialRef->SetFromUserInput("WGS84");
                saveProjection = (char *)"WGS84";
                kmlStyleFile = new string(value);
            }

        } else
            return new string(string("Unable to parse command-line argument ") + item + string("\n\n"));
    }

    if (inDataFile == NULL)
        return new string("no input file specified\n");

    if (cutpointsFile == NULL)
        return new string("no cutpoints file specified\n0");

    if (outFile == NULL)
        return new string("no output file specified\n");

    return NULL;
}






IRaster* ingestGDALRaster()
{
    GDALDataset* ds = gdalDataset;
    cout << "Reading raster metadata...";

    GDALRasterBand* band = ds->GetRasterBand(bandNum);
    int xSize = band->GetXSize();
    int ySize = band->GetYSize();
    int hasNoDataValue;
    double noDataValue = band->GetNoDataValue(&hasNoDataValue);
    if (hasNoDataValue != 0)
        noDataValue = NULL_DOUBLE_;
    double xForm[6];
    ds->GetGeoTransform(xForm);
    double minX = xForm[0];
    double cellSizeX = xForm[1];
    double skewX = xForm[2];
    double minY = xForm[3];
    double skewY = xForm[4];
    double cellSizeY = xForm[5];
    string* spatialRef = new string(ds->GetProjectionRef());

    if( ds->GetMetadataItem("NC_GLOBAL#IOAPI_VERSION", "") != NULL) {
        // Get georeference from IOAPI metadata
        // See: http://www.baronams.com/products/ioapi/GRIDS.html#horiz

        // Build the affine transform from metadata
        minX = atof(ds->GetMetadataItem("NC_GLOBAL#XORIG", ""));
        minY = atof(ds->GetMetadataItem("NC_GLOBAL#YORIG", ""));
        cellSizeX = atof(ds->GetMetadataItem("NC_GLOBAL#XCELL", ""));
        cellSizeY = atof(ds->GetMetadataItem("NC_GLOBAL#YCELL", ""));
        skewX = 0;
        skewY = 0;

        // Build the SpatialReference
        double xcent, ycent, p_alp, p_bet, p_gam;
        char *gdnam;
        OGRSpatialReference* sref = new OGRSpatialReference("");
        // Assume datum is WGS84 (may not be, but IO/API files don't (can't?) say...)
        sref->SetWellKnownGeogCS("WGS84");

        int gdtyp = atoi(ds->GetMetadataItem("NC_GLOBAL#GDTYP", ""));
        switch(gdtyp) {
            case 0:
                // Unknown projection (we assume lat-lon)
                break;

            case 1:
                // LATGRD3 -- Latitude/longitude
                break;

            case 2:
                // LAMGRD3 -- Lambert Conformal Conic (two standard parallels)
                xcent = atof(ds->GetMetadataItem("NC_GLOBAL#XCENT", ""));
                ycent = atof(ds->GetMetadataItem("NC_GLOBAL#YCENT", ""));
                p_alp = atof(ds->GetMetadataItem("NC_GLOBAL#P_ALP", ""));
                p_bet = atof(ds->GetMetadataItem("NC_GLOBAL#P_BET", ""));
                sref->SetLCC(p_alp, p_bet, ycent, xcent, 0, 0);
                gdnam = (char *)ds->GetMetadataItem("NC_GLOBAL#GDNAM", "");
                sref->SetProjCS(gdnam);
                break;
                
            case 9:
                // ALBGRD3 -- Albers Equal-Area Conic
                xcent = atof(ds->GetMetadataItem("NC_GLOBAL#XCENT", ""));
                ycent = atof(ds->GetMetadataItem("NC_GLOBAL#YCENT", ""));
                p_alp = atof(ds->GetMetadataItem("NC_GLOBAL#P_ALP", ""));
                p_bet = atof(ds->GetMetadataItem("NC_GLOBAL#P_BET", ""));
                sref->SetACEA(p_alp, p_bet, ycent, xcent, 0, 0);
                gdnam = (char *)ds->GetMetadataItem("NC_GLOBAL#GDNAM", "");
                sref->SetProjCS(gdnam);
                break;
                
            case 10:
                // LEQGRID3 -- Lambert Azimuthal Equal-Area
                p_alp = atof(ds->GetMetadataItem("NC_GLOBAL#P_ALP", ""));
                // Correct for bad metadata on some files
                if(p_alp == 0.0) {
                    xcent = atof(ds->GetMetadataItem("NC_GLOBAL#XCENT", ""));
                    ycent = atof(ds->GetMetadataItem("NC_GLOBAL#YCENT", ""));
                    p_alp = ycent;
                    p_gam = xcent;
                } else {
                    p_gam = atof(ds->GetMetadataItem("NC_GLOBAL#P_GAM", ""));
                }
                sref->SetLAEA(p_alp, p_gam, 0, 0);
                gdnam = (char *)ds->GetMetadataItem("NC_GLOBAL#GDNAM", "");
                sref->SetProjCS(gdnam);
                break;

            default:
                throw new runtime_error("ERROR: Unable to parse IO/API GDTYP variable");
        }

        char* wktSrStr = new char[spatialRef->length()];
        strcpy((char *)spatialRef->c_str(), wktSrStr);
        sref->exportToWkt(&wktSrStr);
        //CPLFree(sref);
        spatialRef->assign(wktSrStr);
    }

    cout << "...Done.\nReading raster band " << bandNum << "...";

    IRaster* result = NULL;
    CPLErr retval;

    switch (band->GetRasterDataType()) {

        //retval = band->RasterIO(GF_Read, 0, 0, band->XSize, band->YSize, floatArray, band->XSize, band->YSize, 0, 0);


        case GDT_Float32: {
            float* floatArray = new float[xSize * ySize];
            retval = band->RasterIO(GF_Read, 0, 0, xSize, ySize, floatArray, xSize, ySize, band->GetRasterDataType(), 0, 0);
            if (retval != CE_None)
                throw new runtime_error("GDALRasterBand::ReadBlock() returned error");
            result = new Raster<float>(floatArray, xSize, ySize, cellSizeX, cellSizeY, minX, minY, skewX, skewY, spatialRef, noDataValue);
            cout << " -- Pixel type: Float32 -- ...Done\n";
        } break;

        case GDT_Float64: {
            double* doubleArray = new double[xSize * ySize];
            retval = band->RasterIO(GF_Read, 0, 0, xSize, ySize, doubleArray, xSize, ySize, band->GetRasterDataType(), 0, 0);
            if (retval != CE_None)
                throw new runtime_error("GDALRasterBand::ReadBlock() returned error");
            result = new Raster<double>(doubleArray, xSize, ySize, cellSizeX, cellSizeY, minX, minY, skewX, skewY, spatialRef, noDataValue);
            cout << " -- Pixel type: Float64 -- ...Done\n";
        } break;

        case GDT_Int32: {
            int* intArray = new int[xSize * ySize];
            retval = band->RasterIO(GF_Read, 0, 0, xSize, ySize, intArray, xSize, ySize, band->GetRasterDataType(), 0, 0);
            if (retval != CE_None)
                throw new runtime_error("GDALRasterBand::ReadBlock() returned error");
            result = new Raster<int>(intArray, xSize, ySize, cellSizeX, cellSizeY, minX, minY, skewX, skewY, spatialRef, noDataValue);
            cout << " -- Pixel type: Int32 -- ...Done\n";
        } break;

        case GDT_Int16: {
            short* shortArray = new short[xSize * ySize];
            retval = band->RasterIO(GF_Read, 0, 0, xSize, ySize, shortArray, xSize, ySize, band->GetRasterDataType(), 0, 0);
            if (retval != CE_None)
                throw new runtime_error("GDALRasterBand::ReadBlock() returned error");
            result = new Raster<short>(shortArray, xSize, ySize, cellSizeX, cellSizeY, minX, minY, skewX, skewY, spatialRef, noDataValue);
            cout << " -- Pixel type: Int32 -- ...Done\n";
        } break;

        case GDT_Byte: {
            char* byteArray = new char[xSize * ySize];
            retval = band->RasterIO(GF_Read, 0, 0, xSize, ySize, byteArray, xSize, ySize, band->GetRasterDataType(), 0, 0);
            if (retval != CE_None)
                throw new runtime_error("GDALRasterBand::ReadBlock() returned error");
            result = new Raster<char>(byteArray, xSize, ySize, cellSizeX, cellSizeY, minX, minY, skewX, skewY, spatialRef, noDataValue);
            cout << " -- Pixel type: Byte -- ...Done\n";
        } break;

        default:
            throw new runtime_error("Unsupported pixel type");
    }

    return result;
}



OGRGeometry* BuildOgrPolygon(Polygon* poly, OGRCoordinateTransformation* transform)
{
    OGRPolygon* result = new OGRPolygon();

    list<Point*>* pOuter = poly->outerRing;
    list<Point*>::iterator ringIter;
    OGRLinearRing* ring = new OGRLinearRing();
    for (ringIter = pOuter->begin(); ringIter != pOuter->end(); ringIter++) {
        double x = (*ringIter)->X;
        double y = (*ringIter)->Y;
        if (transform != NULL)
            transform->Transform(1, &x, &y);
        ring->addPoint(x, y);
    }
    result->addRingDirectly(ring);

    list<list<Point *>*>* innerRings = poly->innerRings;
    list<list<Point *>*>::iterator ringsIter;
    for (ringsIter = innerRings->begin(); ringsIter != innerRings->end(); ringsIter++) {
        ring = new OGRLinearRing();
        list<Point*>* currRing = *ringsIter;
        for (ringIter = currRing->begin(); ringIter != currRing->end(); ringIter++) {
            double x = (*ringIter)->X;
            double y = (*ringIter)->Y;
            if (transform != NULL)
                transform->Transform(1, &x, &y);
            ring->addPoint(x, y);
        }
        result->addRingDirectly(ring);
    }

    result->closeRings();
    return result;
}



void writePolygons(RegionMap* regionMap)
{
    char* driverName = (char *)ogrDriver->c_str();
    char* outFilePath =  (char *)outFile->c_str();

    cout << "Creating output file \"" << outFilePath << "\" (\"" << driverName << "\" format)...";
    OGRSFDriver* driver = (OGRSFDriver*)OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(ogrDriver->c_str());
    if (driver == NULL) {
        localExit(new string(string("FATAL: OGR driver \"") + string(driverName) +  string("\" is not available.\n")), 1);
    }
    
    char* dataSourceName = NULL;
    bool cleanupDataSource = false;
    if (ogrDriver->compare("KML") == 0 && kmlStyleFile != NULL) {
        dataSourceName = tmpnam(NULL);
        //mkstemp(dataSourceName);
        cleanupDataSource = true;
    }
    else if (ogrDriver->compare("ESRI Shapefile") == 0) {
        string* dsStr = removeFromLastOccurrence(outFilePath, '/');
        dsStr = removeFromLastOccurrence((char *)dsStr->c_str(), '\\');
        dataSourceName = (char *)dsStr->c_str();
    } else {
        dataSourceName = outFilePath;
    }
    
    OGRDataSource* ds = driver->CreateDataSource(dataSourceName);
    if(ds == NULL) {
        localExit(new string(string("FATAL: Unable to create output file \"") + string(outFilePath) + string("\"\n")), 1);
    }
    
    cout << "[ok]\n";

    OGRCoordinateTransformation* transform = NULL;
    const string* rmSpatialRefStr = regionMap->getSpatialRef();
    if (rmSpatialRefStr != NULL && rmSpatialRefStr->compare("") != 0) {
        OGRSpatialReference* spatialRef = new OGRSpatialReference(rmSpatialRefStr->c_str());
        transform = OGRCreateCoordinateTransformation(spatialRef, outSpatialRef);
    }

    char* layerName = (char *)pointToFilename(outFilePath, true)->c_str();
    OGRLayer* layer = ds->CreateLayer(layerName, outSpatialRef, wkbPolygon);
    layer->CreateField(new OGRFieldDefn("RegionID", OFTInteger), 0);
    layer->CreateField(new OGRFieldDefn("Category", OFTString), 0);
    layer->CreateField(new OGRFieldDefn("Threshold", OFTReal), 0);

    cout << "Writing polygon data to file...";
    int count = 0;
    int currRegionID = 0;
    int numRegions =  regionMap->getNumRegions();
    while (currRegionID < numRegions) {

        Region *currRegion = regionMap->getRegion(currRegionID);
        Polygon* poly = currRegion->toPolygon();
        if(poly == NULL) {
            currRegionID += 1;
            continue;
        }
        count++;

        OGRFeature* feature = new OGRFeature(layer->GetLayerDefn());
        feature->SetField(feature->GetFieldIndex("RegionID"), currRegionID);
        feature->SetField(feature->GetFieldIndex("Category"), currRegion->getCategory()->getName()->c_str());
        feature->SetField(feature->GetFieldIndex("Threshold"), currRegion->getCategory()->minThreshold());

        OGRGeometry* geom = BuildOgrPolygon(poly, transform);
        feature->SetGeometry(geom);

        if (layer->CreateFeature(feature) != OGRERR_NONE)
            localExit(new string("ERROR: Unable to create feature for region #\n"), 2);

        //OGRFeature::DestroyFeature(feature);
        currRegionID += 1;

        if (currRegionID % 100 == 0)
            cout << ".";
    }
    //CPLFree(transform);
    //CPLFree(layer);
    OGRDataSource::DestroyDataSource(ds);
    cout << "[ok] Wrote "<< count << " polygons\n";

    if(kmlStyleFile != NULL) {
#ifdef USE_XALAN
        XALAN_USING_XERCES(XMLPlatformUtils)
        XALAN_USING_XALAN(XalanTransformer)
        XMLPlatformUtils::Initialize();
        XalanTransformer::initialize();
        XalanTransformer theXalanTransformer;
        cout << "Transforing KML file ..." << outFile->c_str();
        if(theXalanTransformer.transform(dataSourceName, kmlStyleFile->c_str(), outFile->c_str()) != 0) {
            cout << "[!!]\n";
            cout << "ERROR doing XSLT transform using " << kmlStyleFile->c_str() << "\n";
        } else {
            cout << "...[ok]\n";
        }
#endif /* USE_XALAN */
#ifdef USE_LIBXSLT
        xsltStylesheetPtr cur = NULL;
        xmlDocPtr doc, res;
        const char *params[16 + 1];
        params[0] = NULL;
        xmlSubstituteEntitiesDefault(1);
        xmlLoadExtDtdDefaultValue = 1;
        cur = xsltParseStylesheetFile((const xmlChar *) kmlStyleFile->c_str());
        doc = xmlParseFile(dataSourceName);
        res = xsltApplyStylesheet(cur, doc, params);
        FILE* f = fopen(outFile->c_str(), "w");
        xsltSaveResultToFile(f, res, cur);
        fclose(f);

        xsltFreeStylesheet(cur);
        xmlFreeDoc(res);
        xmlFreeDoc(doc);

        xsltCleanupGlobals();
        xmlCleanupParser();
#endif /* USE_LIBXSLT */
    }
    if(cleanupDataSource) {
        if(!remove(dataSourceName)) {
            fprintf(stderr, "Error deleting temporary file: %s\n", dataSourceName);
        }
    }
}




void resolveProjection(OGRSpatialReference* sr, const char* srcOrOut)
{
    const char* inproj = "null";
    if (sr != NULL) {
        const char* projectionRequest = sr->IsProjected() == 1 ? "PROJCS" : "GEOGCS";
        inproj = sr->GetAttrValue(projectionRequest);
        if (inproj == NULL)
            sr->exportToProj4((char **)&inproj);
    }
    cout << srcOrOut << " projection: " << inproj <<"\n";
}



void polygonize()
{
    IRaster* inRaster = ingestGDALRaster();

    if (inSpatialRef == NULL) {
        if (inRaster->getSpatialRef()->compare("") != 0)
            inSpatialRef = new OGRSpatialReference(inRaster->getSpatialRef()->c_str());
    } else {
        char* wkt;
        inSpatialRef->exportToWkt(&wkt);
        inRaster->setSpatialRef(new string(wkt));
    }

    if (outSpatialRef == NULL)
        outSpatialRef = new OGRSpatialReference(gdalDataset->GetProjectionRef());

    if (noDataValue != NULL_DOUBLE_)
        gdalDataset->GetRasterBand(bandNum)->SetNoDataValue(noDataValue);

    RegionMap* regionMap = new RegionMap(new ThresholdRaster(inRaster, cutpoints));

    resolveProjection(inSpatialRef, "Source");
    resolveProjection(outSpatialRef, "Output");

    writePolygons(regionMap);

    delete regionMap;
    delete inRaster;
}



int localExit(string* msg, int retCode)
{
    cout << msg->c_str();
    //cout << "\nHIT ENTER...\n";
    //char inStr[20];
    //cin.getline(inStr, 20);

    delete msg;

    if (inDataFile != NULL)
        delete inDataFile;
    if (cutpointsFile != NULL)
        delete cutpointsFile;
    if (outFile != NULL)
        delete outFile;
    //if (inSpatialRef != NULL)
        //delete inSpatialRef;
    //if (outSpatialRef != NULL)
        //CPLFree(outSpatialRef);
    if (ogrDriver != NULL)
        delete ogrDriver;
    if (kmlStyleFile != NULL)
        delete kmlStyleFile;
    if (cutpoints != NULL)
        delete cutpoints;
    //if (gdalDataset != NULL)
        //CPLFree(gdalDataset);

    exit(retCode);
}




int main(int argc, const char* argv[])
{
    string* errorMsg = parseCommandLine(argc, argv);
    if (errorMsg != NULL)
        localExit(errorMsg, 0);

    cutpoints = readCutpoints();
    if (cutpoints == NULL)
        localExit(new string("error loading cutpoints\n"), 0);

    if (!initGDAL())
        localExit(new string("GDAL init failed\n"), 0);

    gdalDataset = openGDALDataSet();
    if (gdalDataset == NULL)
        localExit(new string("error loading raster\n"), 0);

    polygonize();

    localExit(new string("\nfinished MakePolygons_C++\n\n"), 0);
}

