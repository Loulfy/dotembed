namespace ManagedLibB;

public static class Lib
{
    public static int Hello(IntPtr arg, int argLength)
    {
        Console.WriteLine($"Hello, world! from {typeof(Lib).Namespace}");
        return 20;
    }
}