#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <map>

struct Parameter
{
	std::string Type;
	std::string Name;
};

struct Method
{
	std::string ReturnType;
	std::string Name;
	std::vector<Parameter> Parameters;
	bool IsConstructor;
	bool IsReturnTypeConst;

	Method()
	{
		IsConstructor = false;
		IsReturnTypeConst = false;
	}
};

struct MethodCall
{
	std::string InstanceName;
	std::string MethodName;
	std::vector<std::string> Parameters;

	std::string ToString() const
	{
		std::string doc;

		if (InstanceName.length() > 0)
		{
			doc.append(InstanceName);
		}

		if (InstanceName.length() > 0 && MethodName.length() > 0)
			doc += "->";

		if (MethodName.length() > 0)
		{
			doc += MethodName + "(";

			std::vector<std::string>::const_iterator itrInputs = Parameters.begin();
			while (itrInputs != Parameters.end())
			{
				doc.append((*itrInputs));
				itrInputs++;

				if (itrInputs != Parameters.end())
					doc.append(", ");
			}

			doc.append(")");
		}

		doc.append(";");

		return doc.append("\n");
	}
};

struct StaticCast
{
	std::string FromName;
	std::string ToName;
	std::string ToType;
	std::string ToString()
	{
		return ToType + " " + ToName + " = static_cast<" + ToType + ">(" + FromName + ");";
	}
};

struct CDeclWrapperMethod
{
	std::string ReturnType;
	std::string ClassName;
	std::string MethodName;
	std::vector<Parameter> Parameters;
	std::vector<StaticCast> StaticCasts;
	std::string InnerCallPrefix;
	MethodCall InnerCall;

	std::string GetMethodName() const
	{
		return MethodName.length() > 0 ? MethodName : InnerCall.MethodName;
	}

	std::string ToString(int postfix = 0) const
	{
		std::string doc;
		doc.append(std::string(R"(extern "C" __declspec(dllexport))") + "\n");

		std::string methodName = GetMethodName();

		if (postfix != 0)
			methodName.append(std::to_string(postfix));

		doc.append(ReturnType + " " + ClassName + "_" + methodName + "(");

		std::vector<Parameter>::const_iterator itrParam = Parameters.begin();
		while (itrParam != Parameters.end())
		{
			Parameter currParam = (*itrParam);
			std::string extParamType = currParam.Type;

			doc.append(extParamType + " " + currParam.Name);

			itrParam++;
			if (itrParam != Parameters.end())
				doc.append(", ");
		}

		doc.append(")\n");
		doc.append("{\n");

		for (StaticCast currCast : StaticCasts)
		{
			doc.append("\t" + currCast.ToString() + "\n");
		}

		doc.append("\t" + InnerCallPrefix + InnerCall.ToString());
		doc.append("}");

		return doc;
	}
};

std::vector<std::string> extractConstructors(std::string className, std::string document)
{
	std::regex constructorExpression(R"(\s()" + className + R"(\(.*\);))",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::sregex_iterator constructorItr = std::sregex_iterator(document.begin(),
		document.end(), constructorExpression);

	std::vector<std::string> constructorLines;
	std::sregex_iterator constructorEnd;
	while (constructorItr != constructorEnd)
	{
		std::string methodStr = constructorItr->operator[](1).str();
		constructorLines.push_back(methodStr + "\n");
		constructorItr++;
	}

	return constructorLines;
}

std::vector<Parameter> parseParameters(std::string line, std::string className, std::string classScope)
{
	size_t lParenPos = line.find('(');
	size_t rParenPos = line.find_last_of(')') + 1;
	std::string paramScope = line.substr(lParenPos, rParenPos - lParenPos);

	// Update parameter extraction to grab namespace

	std::vector<Parameter> results;
	std::regex expParameters(R"((?:const\s+)?(?:class\s+)?([\w:]+\s?[&*]?)(?:\s+(\w+))?(?:[\w\W]*?,|[\w\W]*\)))",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::sregex_iterator paramItr(paramScope.begin(), paramScope.end(), expParameters);
	int paramNumber = 1;
	while (paramItr != std::sregex_iterator())
	{
		Parameter currParam;
		currParam.Type = (*paramItr)[1].str();

		currParam.Name = (*paramItr)[2].matched
			? (*paramItr)[2].str()
			: std::string("param") + std::to_string(paramNumber);

		paramItr++;
		paramNumber++;

		// Create a testing version without reference
		std::string currType = currParam.Type;
		if (currType.back() == '&' || currType.back() == '*')
			currType = currType.substr(0, currType.length() - 1);

		// If the parameter is a struct declared in the class, just add scope to type
		if (classScope.find(std::string("struct ") + currType) != std::string::npos)
			currParam.Type = className + "::" + currParam.Type;

		results.push_back(currParam);
	}

	return results;
}

std::vector<Method> parseConstructors(std::string className, std::string classScope)
{
	std::vector<Method> results;
	std::vector<std::string> methods = extractConstructors(className, classScope);
	std::vector<std::string>::const_iterator funcItr = methods.begin();
	while (funcItr != methods.end())
	{
		std::string currLine = (*funcItr);

		Method currMethod;
		currMethod.Name = className;
		currMethod.Parameters = parseParameters(currLine, className, classScope);
		currMethod.IsConstructor = true;
		
		funcItr++;
		results.push_back(currMethod);
	}

	return results;
}

std::vector<std::string> extractMethods(std::string document)
{
	std::regex functionExpression(R"((?:\w+\ +)*?((?:const )?(?:\w+[\*\&]?)\ +\w+\(.*\))(?:\ +\w+\s*)?\s*[;{])",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::sregex_iterator funcsItr = std::sregex_iterator(document.begin(),
		document.end(), functionExpression);

	std::vector<std::string> functionLines;
	std::sregex_iterator funcsEnd;
	while (funcsItr != funcsEnd)
	{
		std::string methodStr = funcsItr->operator[](1).str();
		functionLines.push_back(methodStr + "\n");
		funcsItr++;
	}

	return functionLines;
}

std::vector<Method> parseMethods(std::string document, std::string className, std::string classScope)
{
	std::regex funcReturnTypeAndNameExtractor(R"((\w+[&*]?)?[\t ]+(\w+)\()",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::regex funcReturnTypeConstChecker(R"(const\s+.+\()",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::vector<Method> results;
	std::vector<std::string> methods = extractMethods(document);
	std::vector<std::string>::const_iterator funcItr = methods.begin();
	while (funcItr != methods.end())
	{
		std::string currLine = (*funcItr);

		std::smatch match;
		if (!std::regex_search(currLine, match, funcReturnTypeAndNameExtractor))
		{
			funcItr++;
			continue;
		}

		Method currMethod;
		
		if (std::regex_search(currLine, funcReturnTypeConstChecker))
			currMethod.IsReturnTypeConst = true;

		currMethod.ReturnType = match[1].str();
		currMethod.Name = match[2].str();
		currMethod.Parameters = parseParameters(currLine, className, classScope);

		results.push_back(currMethod);
		funcItr++;
	}

	return results;
}

// this assumes that we wont need to trim before the scope starts
std::string extractScope(std::string document)
{
	int leftCount = 0;
	int rightCount = 0;
	int startOffset = 0;

	std::string::const_iterator itr = document.begin();
	int index = 0;
	while (itr != document.end())
	{
		if ((*itr) == '{')
		{
			if (leftCount == 0)
				startOffset = index;

			leftCount++;
		}

		if ((*itr) == '}')
			rightCount++;

		if (leftCount > 0 && leftCount == rightCount)
		{
			return document.substr(startOffset, index);
		}

		index++;
		itr++;
	}

	return document;
}

std::vector<Method> extractClass(std::string document, std::string& outClassName)
{
	std::regex classExpression(R"((?:ATTRIBUTE_ALIGNED16\(class\)|class)\s+(\w+).*\n?\{)",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::sregex_iterator classesItr = std::sregex_iterator(document.begin(),
		document.end(), classExpression);

	if (classesItr == std::sregex_iterator())
		return std::vector<Method>();

	std::sregex_iterator::value_type match = (*classesItr);
	outClassName = match[1].str();

	std::string scope = extractScope(document.substr(match.position()));

	// We cant use captures when trying to extract from large pieces of text. []*
	std::regex protectedExp(R"(protected\:[\s\S]*public\:)",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	scope = std::regex_replace(scope, protectedExp, "");

	std::regex definedExp(R"(#if defined(?:.*\n)*?\s*#endif)",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	scope = std::regex_replace(scope, definedExp, "");

	std::vector<Method> methods = parseConstructors(outClassName, scope);
		
	for (Method currMethod : parseMethods(scope, outClassName, scope))
	{
		methods.push_back(currMethod);
	}

	return methods;
}

CDeclWrapperMethod buildCWrapper(std::string className, const Method sourceMethod)
{
	CDeclWrapperMethod currWrapper;

	std::vector<Parameter>::const_iterator itrParam = sourceMethod.Parameters.begin();
	while (itrParam != sourceMethod.Parameters.end())
	{
		Parameter currParam = (*itrParam);
		std::string extParamType = currParam.Type;

		if (extParamType == "btScalar")
			extParamType = "float";

		if (extParamType.find('*') != std::string::npos)
		{
			extParamType = "void*";

			StaticCast instanceCast;
			instanceCast.FromName = currParam.Name;
			instanceCast.ToName = currParam.Name + "Cast";
			instanceCast.ToType = currParam.Type;
			currWrapper.StaticCasts.push_back(instanceCast);
			currWrapper.InnerCall.Parameters.push_back(instanceCast.ToName);
		}
		else if (extParamType == "btVector3&")
		{
			extParamType = "btVector3*";
			currWrapper.InnerCall.Parameters.push_back("(*" + currParam.Name + ")");
		}
		else if (extParamType == "btQuaternion&")
		{
			extParamType = "btQuaternion*";
			currWrapper.InnerCall.Parameters.push_back("(*" + currParam.Name + ")");
		}
		else if (extParamType == "btTransform&")
		{
			extParamType = "btTransform*";
			currWrapper.InnerCall.Parameters.push_back("(*" + currParam.Name + ")");
		}
		else if (extParamType == "btRigidBodyConstructionInfo&")
		{
			extParamType = "btRigidBodyConstructionInfo*";
			currWrapper.InnerCall.Parameters.push_back("(*" + currParam.Name + ")");
		}
		else
		{
			currWrapper.InnerCall.Parameters.push_back(currParam.Name);
		}

		Parameter param;
		param.Name = currParam.Name;
		param.Type = extParamType;
		currWrapper.Parameters.push_back(param);

		itrParam++;
	}

	Parameter objInstanceParam;
	objInstanceParam.Name = "objectPtr";
	objInstanceParam.Type = "void*";
	currWrapper.Parameters.insert(currWrapper.Parameters.begin(), objInstanceParam);

	StaticCast instanceCast;
	instanceCast.FromName = objInstanceParam.Name;
	instanceCast.ToName = className + "Ptr";
	instanceCast.ToType = className + "*";
	currWrapper.StaticCasts.push_back(instanceCast);

	currWrapper.ClassName = className;
	currWrapper.ReturnType = sourceMethod.ReturnType;
	currWrapper.InnerCall.MethodName = sourceMethod.Name;
	currWrapper.InnerCall.InstanceName = instanceCast.ToName;

	if (currWrapper.ReturnType.find('*') != std::string::npos)
	{
		currWrapper.ReturnType = "void*";
		currWrapper.InnerCallPrefix = "return ";
	}

	if (currWrapper.ReturnType == "btScalar")
	{
		currWrapper.ReturnType = "float";
		currWrapper.InnerCallPrefix = "return ";
	}

	if (currWrapper.ReturnType == "bool")
		currWrapper.InnerCallPrefix = "return ";

	if (currWrapper.ReturnType == "int")
		currWrapper.InnerCallPrefix = "return ";

	if (currWrapper.ReturnType == "btVector3" || currWrapper.ReturnType == "btVector3&")
	{
		currWrapper.ReturnType = "void";

		Parameter objInstanceParam;
		objInstanceParam.Name = "btVector3Out";
		objInstanceParam.Type = "btVector3*";
		currWrapper.Parameters.push_back(objInstanceParam);
		currWrapper.InnerCallPrefix = "(*" + objInstanceParam.Name + ") = ";
	}

	if (currWrapper.ReturnType == "btQuaternion" || currWrapper.ReturnType == "btQuaternion&")
	{
		currWrapper.ReturnType = "void";

		Parameter objInstanceParam;
		objInstanceParam.Name = "btQuaternionOut";
		objInstanceParam.Type = "btQuaternion*";
		currWrapper.Parameters.push_back(objInstanceParam);
		currWrapper.InnerCallPrefix = "(*" + objInstanceParam.Name + ") = ";
	}

	if (currWrapper.ReturnType == "btMatrix3x3" || currWrapper.ReturnType == "btMatrix3x3&")
	{
		currWrapper.ReturnType = "void";

		Parameter objInstanceParam;
		objInstanceParam.Name = "btMatrix3x3Out";
		objInstanceParam.Type = "btMatrix3x3*";
		currWrapper.Parameters.push_back(objInstanceParam);
		currWrapper.InnerCallPrefix += "(*" + objInstanceParam.Name + ") = ";
	}

	if (currWrapper.ReturnType == "btTransform" || currWrapper.ReturnType == "btTransform&")
	{
		currWrapper.ReturnType = "void";

		Parameter objInstanceParam;
		objInstanceParam.Name = "btTransformOut";
		objInstanceParam.Type = "btTransform*";
		currWrapper.Parameters.push_back(objInstanceParam);
		currWrapper.InnerCallPrefix += "(*" + objInstanceParam.Name + ") = ";
	}

	if (currWrapper.ReturnType == "btDynamicsWorldType")
	{
		currWrapper.ReturnType = "void";

		Parameter objInstanceParam;
		objInstanceParam.Name = "btDynamicsWorldTypeOut";
		objInstanceParam.Type = "btDynamicsWorldType*";
		currWrapper.Parameters.push_back(objInstanceParam);
		currWrapper.InnerCallPrefix += "(*" + objInstanceParam.Name + ") = ";
	}

	if (sourceMethod.IsConstructor)
	{
		currWrapper.MethodName = "create";
		currWrapper.ReturnType = "void*";
		currWrapper.InnerCallPrefix = "return new ";
		currWrapper.InnerCall.InstanceName = "";
	}

	if (sourceMethod.IsReturnTypeConst)
		currWrapper.ReturnType = std::string("const ") + currWrapper.ReturnType;

	return currWrapper;
}

int main(int argc, char* argv[])
{
	std::vector<std::string> args;

	for (int i = 0; i < argc; ++i)
		args.push_back(std::string(argv[i])); 

	std::string outputDoc;

	std::string includePath(args[2]);
	std::ifstream inputFile(args[1] + args[2]);
	std::ofstream outputFile(args[3]);

	std::string originDoc;
	originDoc.assign(std::istreambuf_iterator<char>(inputFile),
					 std::istreambuf_iterator<char>());

	std::string className;
	std::vector<Method> methods = extractClass(originDoc, className);

	std::vector<CDeclWrapperMethod> wrappers;
	for (const Method currMethod : methods)
	{
		wrappers.push_back(buildCWrapper(className, currMethod));
	}

	CDeclWrapperMethod destructor;
	destructor.ClassName = className;
	destructor.ReturnType = "void";
	destructor.InnerCallPrefix = "delete ";
	destructor.MethodName = "destroy";
	
	Parameter objInstanceParam;
	objInstanceParam.Name = "objectPtr";
	objInstanceParam.Type = "void*";
	destructor.Parameters.insert(destructor.Parameters.begin(), objInstanceParam);

	StaticCast instanceCast;
	instanceCast.FromName = objInstanceParam.Name;
	instanceCast.ToName = className + "Ptr";
	instanceCast.ToType = className + "*";
	destructor.StaticCasts.push_back(instanceCast);
	destructor.InnerCall.InstanceName = instanceCast.ToName;
	
	wrappers.insert(wrappers.begin(), destructor);
	
	outputDoc.append("// GENERATED, DO NOT EDIT\n");
	outputDoc.append("\n");
	outputDoc.append(std::string("#include \"") + includePath + "\"\n");

	std::map<std::string, int> nameCounts;

	for (const CDeclWrapperMethod currWrapper : wrappers)
	{
		if (nameCounts.find(currWrapper.GetMethodName()) == nameCounts.end())
			nameCounts[currWrapper.GetMethodName()] = 0;

		nameCounts[currWrapper.GetMethodName()]++;
		int currCount = nameCounts[currWrapper.GetMethodName()];
		
		// Ommiting 1 for aesthetic reasons.
		if (currCount == 1)
			currCount = 0;

		outputDoc.append("\n");
		outputDoc.append(currWrapper.ToString(currCount) + "\n");
	}

	
	outputFile << outputDoc;

	return 0;
}
