"use client";

import { Canvas as ThreeCanvas } from "@react-three/fiber";
import { OrbitControls } from "@react-three/drei";

export function Canvas({ children }: { children: React.ReactNode }) {
    return (
        <ThreeCanvas camera={{ position: [0, 0, -0.2], near: 0.025 }}>
            <ambientLight intensity={Math.PI / 2} />
            <spotLight position={[10, 10, 10]} angle={0.15} penumbra={1} decay={0} intensity={Math.PI} />
            <pointLight position={[-10, -10, -10]} decay={0} intensity={Math.PI} />

            <OrbitControls enableDamping={false} enableZoom={false} enablePan={false} />

            {children}
        </ThreeCanvas>
    );
}
