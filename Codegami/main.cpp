#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <map>
#include <algorithm>

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
	bool IsAbstract;
	bool IsReturnTypeConst;

	Method()
	{
		IsConstructor = false;
		IsAbstract = false;
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

struct Object
{
	std::string Type;
	std::string Name;
	std::vector<Method> Methods;
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

	void addParameter(const Parameter& param)
	{
		std::string extParamType = param.Type;
		extParamType.erase(std::remove(extParamType.begin(), extParamType.end(), ' '), extParamType.end());

		std::string extParamName = param.Name;
		extParamName.erase(std::remove(extParamName.begin(), extParamName.end(), ' '), extParamName.end());

		std::string innerParam = param.Name;
		bool derefInnerParam = false;

		size_t ampIndex = extParamType.find('&');
		if (ampIndex != std::string::npos)
		{
			extParamType.replace(ampIndex, 1, "*");
			derefInnerParam = true;
		}

		if (extParamType == "btScalar")
			extParamType = "float";

		Parameter extParam;
		extParam.Name = extParamName;
		extParam.Type = extParamType;
		Parameters.push_back(extParam);

		if (derefInnerParam)
			innerParam = "(*" + innerParam + ")";

		InnerCall.Parameters.push_back(innerParam);
	}

	static bool IsBasicType(std::string type)
	{
		if (type == "bool")
			return true;

		if (type == "void*")
			return true;

		if (type == "int")
			return true;

		if (type == "float")
			return true;

		return false;
	}

	static CDeclWrapperMethod build(std::string className, const Method sourceMethod)
	{
		CDeclWrapperMethod currWrapper;

		std::vector<Parameter>::const_iterator itrParam = sourceMethod.Parameters.begin();
		while (itrParam != sourceMethod.Parameters.end())
		{
			currWrapper.addParameter((*itrParam));
			itrParam++;
		}

		if (!sourceMethod.IsConstructor)
		{
			Parameter objInstanceParam;
			objInstanceParam.Name = "objectPtr";
			objInstanceParam.Type = "void*";
			currWrapper.Parameters.insert(currWrapper.Parameters.begin(), objInstanceParam);

			StaticCast instanceCast;
			instanceCast.FromName = objInstanceParam.Name;
			instanceCast.ToName = className + "Ptr";
			instanceCast.ToType = className + "*";
			currWrapper.StaticCasts.push_back(instanceCast);
			currWrapper.InnerCall.InstanceName = instanceCast.ToName;
		}

		currWrapper.ClassName = className;
		currWrapper.ReturnType = sourceMethod.ReturnType;
		currWrapper.InnerCall.MethodName = sourceMethod.Name;

		currWrapper.MethodName = std::regex_replace(currWrapper.InnerCall.MethodName, std::regex(R"(=)"), "_assign");
		currWrapper.MethodName = std::regex_replace(currWrapper.MethodName, std::regex(R"(\*)"), "_multiply");
		currWrapper.MethodName = std::regex_replace(currWrapper.MethodName, std::regex(R"(\(\))"), "_call");

		if (currWrapper.ReturnType == "btScalar")
			currWrapper.ReturnType = "float";

		if (currWrapper.ReturnType.find('*') != std::string::npos)
			currWrapper.ReturnType = "void*";

		if (IsBasicType(currWrapper.ReturnType))
		{
			currWrapper.InnerCallPrefix = "return ";
		}
		else if (currWrapper.ReturnType != "void")
		{
			if (currWrapper.ReturnType.back() == '&')
				currWrapper.ReturnType.pop_back();

			Parameter objInstanceParam;
			objInstanceParam.Name = currWrapper.ReturnType + "Out";
			objInstanceParam.Type = currWrapper.ReturnType + "*";

			currWrapper.Parameters.push_back(objInstanceParam);
			currWrapper.InnerCallPrefix = "(*" + objInstanceParam.Name + ") = ";
			currWrapper.ReturnType = "void";
		}

		if (sourceMethod.IsConstructor)
		{
			currWrapper.MethodName = "create";
			currWrapper.ReturnType = "void*";
			currWrapper.InnerCallPrefix = "return new ";
			currWrapper.InnerCall.InstanceName = "";
		}

		if (sourceMethod.IsReturnTypeConst && currWrapper.ReturnType != "void")
			currWrapper.ReturnType = std::string("const ") + currWrapper.ReturnType;

		return currWrapper;
	}
};

std::vector<Parameter> parseParameters(std::string line, std::string className, std::string classScope)
{
	size_t lParenPos = line.find('(');
	size_t rParenPos = line.find_last_of(')') + 1;
	std::string paramScope = line.substr(lParenPos, rParenPos - lParenPos);

	std::vector<Parameter> results;
	std::regex expParameters(R"((?:^\(|,)\s*(?:(?:const|struct|class)\s+)*([^\d\s](?:\w+::)?\w+(?:<.+>)?\s*&?(?:\s*\*)*)\s?(\w+)?(?=(?:,|\)$|\s*=)))",
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
		std::regex structCheckExp(std::string(R"(struct\s*)") + currType + R"(\s*\{)");
		if (std::regex_search(classScope, structCheckExp))
			currParam.Type = className + "::" + currParam.Type;

		results.push_back(currParam);
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

std::string::iterator findScopeEnd(std::string::iterator searchStart, 
	std::string::const_iterator searchEnd)
{
	int leftCount = 0;
	int rightCount = 0;
	int startOffset = 0;

	std::string::iterator& itr = searchStart;
	while (itr != searchEnd)
	{
		if ((*itr) == '{')
			leftCount++;

		if ((*itr) == '}')
			rightCount++;

		if (leftCount > 0 && leftCount == rightCount)
		{
			return itr;
		}

		itr++;
	}

	return searchStart;
}

std::vector<Method> parseMethods(std::string document, std::string className)
{
	std::vector<Method> results;

	// TODO: Check if method is abstrac and mark it, then dont create constructors if class has any abstract methods

	const std::string LINE_START = R"(^[\t ]*)";
	const std::string KEYWORDS = R"((?:\w+\s+)*?)";
	const std::string RETURN_CAPTURE = R"(((?:\w+::)?\w*?[&*]?)\s*)";
	const std::string NAME_CAPTURE = R"(([\w:=]+(?:\(\)|\*)?))";
	const std::string PARAMS_CAPTURE = R"((\([\s\w&,=()*]*\)))";
	const std::string POST_KEYWORDS = R"((?:\s*\w+)*)";
	const std::string TERMINATOR = R"(.*?(?:;|[\s\w:(),-]*\{))";

	std::regex funcDeclExtractor(LINE_START + KEYWORDS + RETURN_CAPTURE 
		+ NAME_CAPTURE + PARAMS_CAPTURE + POST_KEYWORDS + TERMINATOR,
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::regex funcReturnTypeConstChecker(R"(^[\w\s]*const)",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::regex funcAbstractChecker(R"(=\s*0\s*;$)",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::string::iterator searchFront = document.begin();
	std::string::const_iterator searchEnd = document.end();
	
	while (searchFront != searchEnd)
	{
		std::smatch searchMatch;
		// We need to cast to const iterators, which I did not expect. We have to
		// use an iterator we can advance, but the end can be const.
		if (!std::regex_search((std::string::const_iterator)searchFront,
			searchEnd, searchMatch, funcDeclExtractor))
		{
			break;
		}

		std::string& methodDeclLine = searchMatch[0].str();

		Method currMethod;

		if (std::regex_search(methodDeclLine, funcReturnTypeConstChecker))
			currMethod.IsReturnTypeConst = true;

		if (std::regex_search(methodDeclLine, funcAbstractChecker))
			currMethod.IsAbstract = true;

		currMethod.ReturnType = searchMatch[1].str();
		currMethod.Name = searchMatch[2].str();

		if (currMethod.Name == className)
		{
			currMethod.IsConstructor = true;
			currMethod.ReturnType = className + "*";
		}

		currMethod.Parameters = parseParameters(searchMatch[3].str(), className, document);

		results.push_back(currMethod);
		searchFront += searchMatch.position(0) + searchMatch.length() - 1;

		if ((*searchFront) == '{')
			searchFront = findScopeEnd(searchFront, searchEnd);
	}

	return results;
}

std::string filterObjectScope(std::string scope)
{
	std::regex commentExp(R"(\/\*[\w\W]*?\*\/)",
						  std::regex_constants::ECMAScript | std::regex_constants::icase);

	scope = std::regex_replace(scope, commentExp, "");

	std::regex protectedExp(R"(protected\:[\s\S]*?public\:)",
							std::regex_constants::ECMAScript | std::regex_constants::icase);

	scope = std::regex_replace(scope, protectedExp, "");

	std::regex definedExp(R"(#if defined(?:.*\n)*?\s*#endif)",
						  std::regex_constants::ECMAScript | std::regex_constants::icase);

	scope = std::regex_replace(scope, definedExp, "");

	std::regex alignedAllocatorExp(R"(BT_DECLARE_ALIGNED_ALLOCATOR\(\);)",
								   std::regex_constants::ECMAScript | std::regex_constants::icase);

	scope = std::regex_replace(scope, alignedAllocatorExp, "");

	return scope;
}

std::vector<Object> extractObjects(std::string document)
{
	std::regex extractObjectsExp(R"((?:ATTRIBUTE_ALIGNED16\()?(class|struct)\)?\s+(\w+).*\n?\{)",
		std::regex_constants::ECMAScript | std::regex_constants::icase);

	std::vector<Object> results;

	std::string::iterator searchFront = document.begin();
	std::string::const_iterator searchEnd = document.end();

	while (searchFront != searchEnd)
	{
		std::smatch searchMatch;
		// We need to cast to const iterators, which I did not expect. We have to
		// use an iterator we can advance, but the end can be const.
		if (!std::regex_search((std::string::const_iterator)searchFront,
							   searchEnd, searchMatch, extractObjectsExp))
		{
			break;
		}

		Object foundObj;
		foundObj.Type = searchMatch[1].str();
		foundObj.Name = searchMatch[2].str();

		size_t scopeStart = std::distance(document.begin(), searchFront) 
			+ searchMatch.position(0) + searchMatch.length(0) - 1;

		std::string rawScope = extractScope(document.substr(scopeStart));
		std::string scope = filterObjectScope(rawScope);

		foundObj.Methods = parseMethods(scope, foundObj.Name);
		results.push_back(foundObj);

		searchFront += searchMatch.position(0) + searchMatch.length(0) + rawScope.length();
	}

	return results;
}

CDeclWrapperMethod createDefaultConstructor(std::string className)
{
	CDeclWrapperMethod constructor;
	constructor.ClassName = className;
	constructor.ReturnType = className + "*";
	constructor.InnerCallPrefix = "return new ";
	constructor.MethodName = "create";

	constructor.InnerCall.MethodName = className;
	return constructor;
}

CDeclWrapperMethod createDestructor(std::string className)
{
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

	return destructor;
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

	outputDoc.append("// GENERATED, DO NOT EDIT\n");
	outputDoc.append("\n");
	outputDoc.append(std::string("#include \"") + includePath + "\"\n");

	std::vector<Object> allObjects = extractObjects(originDoc);
	for (Object currObj : allObjects)
	{
		// Check for abstractness
		bool isClassAbstract = false;
		bool hasConstructor = false;

		for (const Method currMethod : currObj.Methods)
		{
			if (currMethod.IsAbstract)
				isClassAbstract = true;

			if (currMethod.IsConstructor)
				hasConstructor = true;
		}

		// Filter out constructors if abstract
		std::vector<Method> methods;
		std::copy_if(currObj.Methods.begin(), currObj.Methods.end(), std::back_inserter(methods), [&isClassAbstract](Method currM)
					 { 
		if (isClassAbstract && currM.IsConstructor)
			return false;

		return true; });

		std::vector<CDeclWrapperMethod> wrappers;
		for (const Method currMethod : methods)
		{
			wrappers.push_back(CDeclWrapperMethod::build(currObj.Name, currMethod));
		}

		wrappers.insert(wrappers.begin(), createDestructor(currObj.Name));

		if (!hasConstructor)
			wrappers.insert(wrappers.begin(), createDefaultConstructor(currObj.Name));

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
	}

	outputFile << outputDoc;

	return 0;
}
