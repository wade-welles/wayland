import qbs 1.0

LiriDynamicLibrary {
    name: "materialdecorationplugin"
    targetName: "material"

    Depends { name: "lirideployment" }
    Depends { name: "materialdecoration" }

    condition: {
        if (!project.withMaterialDecoration) {
            console.info("Material decoration disabled");
            return false;
        }

        return true;
    }

    files: ["*.cpp"]

    Group {
        qbs.install: true
        qbs.installDir: lirideployment.pluginsDir + "/wayland-decoration-client"
        fileTagsFilter: ["dynamiclibrary"]
    }
}
