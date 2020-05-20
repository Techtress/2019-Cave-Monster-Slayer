#ifndef AICONTROLERFACTORY_H
#define AICONTROLERFACTORY_H

class XMLTag;
class AIBehavior;
class AIController;

class AIControllerFactory
{
 public:
  static AIControllerFactory &getInstance();

  void registerBehaviorType(const std::string &type, std::function<AIBehavior*(const XMLTag&)>&&);
  AIBehavior *parseBehavior(const std::string &type, const XMLTag&);

  AIController* instantiateController();
  void deactivateController(AIController*);
  
 private:
  AIControllerFactory();
  std::unordered_map<std::string, std::function<AIBehavior*(const XMLTag&)>> behaviorTypes;
};

#endif
