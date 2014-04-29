/**
 * @file   CognitiveMapStorage.h
 * @author David Haensel (d.haensel@fz-juelich.de)
 * @date   February, 2014
 * @brief  Cognitive Map Storage
 *
 */

#ifndef COGNITIVEMAPSTORAGE_H_
#define COGNITIVEMAPSTORAGE_H_

#include <unordered_map>

class Building;
class Pedestrian;
class CognitiveMap;
class AbstractCognitiveMapCreator;


typedef const Pedestrian * CMStorageKeyType;
typedef CognitiveMap * CMStorageValueType;
typedef std::unordered_map<CMStorageKeyType, CMStorageValueType> CMStorageType;



/**
 * @brief Cognitive Map Storage
 *
 * Cares about Cognitive map storage, creation and delivery
 *
 */
class CognitiveMapStorage {
 public:
  CognitiveMapStorage(const Building * const b);
  virtual ~CognitiveMapStorage();


  CMStorageValueType operator[] (CMStorageKeyType key);


 private:
  const Building * const building;
  CMStorageType cognitive_maps;
  AbstractCognitiveMapCreator * creator;


  void CreateCognitiveMap(CMStorageKeyType ped);

};

#endif /* COGNITIVEMAPSTORAGE_H_ */
