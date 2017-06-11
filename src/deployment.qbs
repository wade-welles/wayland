import qbs 1.0

Project {
    name: "Deployment"

    InstallPackage {
        name: "liri-wayland-artifacts"
        targetName: name
        builtByDefault: false

        archiver.type: "tar"
        archiver.outputDirectory: project.buildDirectory

        Depends { name: "Fullscreen Shell Integration" }
        Depends { name: "kms" }
        Depends { name: "liriplatformplugin" }
        Depends { name: "liriwaylandserverplugin" }
        Depends { name: "liriwaylandserverprivateplugin" }
        Depends { name: "lirixwaylandplugin" }
        Depends { name: "LiriEglFS" }
        Depends { name: "LiriLibInput" }
        Depends { name: "LiriLogind" }
        Depends { name: "LiriUDev" }
        Depends { name: "LiriWaylandClient" }
        Depends { name: "LiriWaylandServer" }
        Depends { name: "materialdecorationplugin" }
        Depends { name: "QPA" }
    }
}
