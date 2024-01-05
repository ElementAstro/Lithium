declare interface Module {
  id: number;
  name: string;
  author: string;
  version: string;
  thumbnail: string;
  package: {
    name: string;
    dependencies: Record<string, string>;
  };
}
