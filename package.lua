name "github.com/iolua/iolua"

version "0.0.1"

plugin "github.com/iolua/ik-clang" : localpath "./ik-clang"

property.test = {}

task.install = function (...)
    print("----")
end

