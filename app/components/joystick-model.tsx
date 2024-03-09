"use client";

import { useGLTF, Html } from "@react-three/drei";
import {
    Dialog,
    DialogContent,
    DialogDescription,
    DialogHeader,
    DialogTitle,
    DialogTrigger,
} from "~/components/ui/dialog";

export function JoystickModel() {
    const { scene } = useGLTF("/models/joystick.glb");

    const annotations: React.ReactNode[] = [];

    scene.traverse((o) => {
        if (o.userData.prop) {
            annotations.push(
                <Html key={o.uuid} position={[o.position.x, o.position.y, o.position.z]} distanceFactor={0.25}>
                    <div className="annotation">
                        <Dialog>
                            <DialogTrigger>{o.userData.prop}</DialogTrigger>
                            <DialogContent className="z-[9999999999]">
                                <DialogHeader>
                                    <DialogTitle>Are you absolutely sure?</DialogTitle>
                                    <DialogDescription>
                                        This action cannot be undone. This will permanently delete your account and
                                        remove your data from our servers.
                                    </DialogDescription>
                                </DialogHeader>
                            </DialogContent>
                        </Dialog>
                    </div>
                </Html>,
            );
        }
    });

    return <primitive object={scene}>{annotations}</primitive>;
}
