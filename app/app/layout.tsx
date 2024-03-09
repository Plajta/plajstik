import { Inter as FontSans } from "next/font/google";
import { ThemeSwitcher } from "~/components/theme-switcher";
import { ThemeProvider } from "~/components/theme-provider";
import type { Metadata, Viewport } from "next";

import { cn } from "~/lib/utils";

import "./globals.css";

const fontSans = FontSans({
    subsets: ["latin"],
    variable: "--font-sans",
});

export const viewport: Viewport = {
    themeColor: [
        { media: "(prefers-color-scheme: light)", color: "white" },
        { media: "(prefers-color-scheme: dark)", color: "black" },
    ],
};

export const metadata: Metadata = {
    title: "Plajstick",
    description: "Konfigurator Plajsticku",
};

export default function RootLayout({
    children,
}: Readonly<{
    children: React.ReactNode;
}>) {
    return (
        <html lang="en" suppressHydrationWarning>
            <head />
            <body
                className={cn(
                    "overflow-hidden min-h-screen bg-background font-sans antialiased p-2",
                    fontSans.variable,
                )}
            >
                <ThemeProvider attribute="class" defaultTheme="system" enableSystem>
                    <ThemeSwitcher />

                    {children}
                </ThemeProvider>
            </body>
        </html>
    );
}
