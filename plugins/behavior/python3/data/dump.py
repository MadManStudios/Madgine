def dump(item, depth = 0):
    if type(item) is str:
        return ""
    s = ""   
    try:
        first = False
        for k, v in item:
            if not first:
                first = True
            else:
                s += ",\n"            
            s += (depth + 1) * '\t' + repr(k) + ": " + str(v) + dump(v, depth + 1)    
        return "{\n" + s + "\n" + depth * '\t' + "}"
    except ValueError as e:
        pass
    except TypeError as e:
        pass
    except:
        raise
    s = ""
    try:
        first = False
        for v in item:
            if not first:
                first = True
            else:
                s += ",\n"    
            s += (depth + 1) * '\t' + str(v) + dump(v, depth + 1)
        return "[\n" + s + "\n" + depth * '\t' + "]"
    except TypeError as e:
        pass
    except:
        raise
    return "" 
